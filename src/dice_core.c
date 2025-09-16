#include "dice_core.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>
#include <ctype.h>

// =============================================================================
// Arena Allocator Implementation
// =============================================================================

static void* arena_alloc(dice_context_t *ctx, size_t size) {
    // Align to 8-byte boundary
    size = (size + 7) & ~7;
    
    if (ctx->arena_used + size > ctx->arena_size) {
        snprintf(ctx->error.message, sizeof(ctx->error.message), 
                "Arena allocator out of memory: requested %zu, available %zu", 
                size, ctx->arena_size - ctx->arena_used);
        ctx->error.code = -1;
        ctx->error.has_error = true;
        return NULL;
    }
    
    void *ptr = (char*)ctx->arena + ctx->arena_used;
    ctx->arena_used += size;
    memset(ptr, 0, size);
    return ptr;
}

// =============================================================================
// Context Management
// =============================================================================

dice_context_t* dice_context_create(size_t arena_size, dice_features_t features) {
    dice_context_t *ctx = malloc(sizeof(dice_context_t));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(dice_context_t));
    
    // Allocate arena
    ctx->arena = malloc(arena_size);
    if (!ctx->arena) {
        free(ctx);
        return NULL;
    }
    
    ctx->arena_size = arena_size;
    ctx->arena_used = 0;
    ctx->features = features;
    
    // Set default policy
    ctx->policy = dice_default_policy();
    
    // Initialize with system RNG
    ctx->rng = dice_create_system_rng(0);
    
    return ctx;
}

void dice_context_destroy(dice_context_t *ctx) {
    if (!ctx) return;
    
    // Cleanup RNG if needed
    if (ctx->rng.cleanup) {
        ctx->rng.cleanup(ctx->rng.state);
    }
    
    // Free arena
    free(ctx->arena);
    free(ctx);
}

void dice_context_reset(dice_context_t *ctx) {
    if (!ctx) return;
    
    // Reset arena
    ctx->arena_used = 0;
    
    // Clear error
    ctx->error.has_error = false;
    ctx->error.code = 0;
    ctx->error.message[0] = '\0';
    
    // Clear trace
    memset(&ctx->trace, 0, sizeof(ctx->trace));
}

int dice_context_set_rng(dice_context_t *ctx, const dice_rng_vtable_t *rng_vtable) {
    if (!ctx || !rng_vtable) return -1;
    
    // Cleanup old RNG
    if (ctx->rng.cleanup) {
        ctx->rng.cleanup(ctx->rng.state);
    }
    
    // Copy new RNG
    ctx->rng = *rng_vtable;
    return 0;
}

int dice_context_set_policy(dice_context_t *ctx, const dice_policy_t *policy) {
    if (!ctx || !policy) return -1;
    
    ctx->policy = *policy;
    return 0;
}

// =============================================================================
// Parser Implementation (EBNF Grammar)
// =============================================================================

typedef struct parser_state {
    dice_context_t *ctx;
    const char *input;
    const char *pos;
} parser_state_t;

static void skip_whitespace(parser_state_t *state) {
    while (isspace(*state->pos)) {
        state->pos++;
    }
}

static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}

static bool is_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static dice_ast_node_t* create_node(dice_context_t *ctx, dice_node_type_t type) {
    dice_ast_node_t *node = arena_alloc(ctx, sizeof(dice_ast_node_t));
    if (node) {
        node->type = type;
    }
    return node;
}

// Forward declarations
static dice_ast_node_t* parse_expression(parser_state_t *state);
static dice_ast_node_t* parse_sum(parser_state_t *state);
static dice_ast_node_t* parse_product(parser_state_t *state);
static dice_ast_node_t* parse_unary(parser_state_t *state);
static dice_ast_node_t* parse_primary(parser_state_t *state);

static dice_ast_node_t* parse_number(parser_state_t *state) {
    skip_whitespace(state);
    
    if (!is_digit(*state->pos)) {
        return NULL;
    }
    
    int64_t value = 0;
    while (is_digit(*state->pos)) {
        value = value * 10 + (*state->pos - '0');
        state->pos++;
    }
    
    dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_LITERAL);
    if (node) {
        node->data.literal.value = value;
    }
    return node;
}

static dice_ast_node_t* parse_dice(parser_state_t *state) {
    skip_whitespace(state);
    
    dice_ast_node_t *count = NULL;
    
    // Check if there's a count before 'd'
    const char *saved_pos = state->pos;
    if (is_digit(*state->pos)) {
        count = parse_number(state);
        skip_whitespace(state);
        
        // If no 'd' follows, this isn't a dice expression
        if (*state->pos != 'd' && *state->pos != 'D') {
            state->pos = saved_pos;
            return NULL;
        }
    } else if (*state->pos != 'd' && *state->pos != 'D') {
        return NULL;
    }
    
    // Consume 'd' or 'D'
    if (*state->pos == 'd' || *state->pos == 'D') {
        state->pos++;
    } else {
        return NULL;
    }
    
    // Parse sides - use primary to avoid consuming too much
    dice_ast_node_t *sides = parse_number(state);
    if (!sides) {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Expected number of sides after 'd'");
        state->ctx->error.has_error = true;
        return NULL;
    }
    
    // Check for modifiers (exploding, keep/drop, etc.)
    skip_whitespace(state);
    dice_dice_type_t dice_type = DICE_DICE_BASIC;
    dice_ast_node_t *modifier = NULL;
    
    if (*state->pos == '!') {
        dice_type = DICE_DICE_EXPLODING;
        state->pos++;
    } else if (*state->pos == 'k' || *state->pos == 'K') {
        state->pos++;
        if (*state->pos == 'h' || *state->pos == 'H') {
            dice_type = DICE_DICE_KEEP_HIGH;
            state->pos++;
        } else if (*state->pos == 'l' || *state->pos == 'L') {
            dice_type = DICE_DICE_KEEP_LOW;
            state->pos++;
        } else {
            dice_type = DICE_DICE_KEEP_HIGH; // default to keep high
        }
        modifier = parse_number(state);
    } else if (*state->pos == 'd' || *state->pos == 'D') {
        // Check if this is drop (not another dice)
        if (state->pos[1] == 'h' || state->pos[1] == 'H' || state->pos[1] == 'l' || state->pos[1] == 'L') {
            state->pos++; // consume 'd'
            if (*state->pos == 'h' || *state->pos == 'H') {
                dice_type = DICE_DICE_DROP_HIGH;
                state->pos++;
            } else if (*state->pos == 'l' || *state->pos == 'L') {
                dice_type = DICE_DICE_DROP_LOW;
                state->pos++;
            }
            modifier = parse_number(state);
        }
    }
    
    dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_DICE_OP);
    if (node) {
        node->data.dice_op.dice_type = dice_type;
        node->data.dice_op.count = count;
        node->data.dice_op.sides = sides;
        node->data.dice_op.modifier = modifier;
    }
    
    return node;
}

static dice_ast_node_t* parse_group(parser_state_t *state) {
    skip_whitespace(state);
    
    if (*state->pos != '(') {
        return NULL;
    }
    
    state->pos++; // consume '('
    dice_ast_node_t *expr = parse_expression(state);
    if (!expr) {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Expected expression after '('");
        state->ctx->error.has_error = true;
        return NULL;
    }
    
    skip_whitespace(state);
    if (*state->pos != ')') {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Expected ')' after expression");
        state->ctx->error.has_error = true;
        return NULL;
    }
    
    state->pos++; // consume ')'
    return expr;
}

static dice_ast_node_t* parse_primary(parser_state_t *state) {
    skip_whitespace(state);
    
    dice_ast_node_t *result = NULL;
    
    // Try parsing in order: group, dice, number
    if (*state->pos == '(') {
        result = parse_group(state);
    } else if (is_digit(*state->pos) || *state->pos == 'd' || *state->pos == 'D') {
        // Try dice first, then number
        const char *saved_pos = state->pos;
        result = parse_dice(state);
        if (!result && !state->ctx->error.has_error) {
            state->pos = saved_pos;
            result = parse_number(state);
        }
    }
    
    if (!result && !state->ctx->error.has_error) {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Expected number, dice expression, or parenthesized expression");
        state->ctx->error.has_error = true;
    }
    
    return result;
}

static dice_ast_node_t* parse_unary(parser_state_t *state) {
    skip_whitespace(state);
    
    if (*state->pos == '+' || *state->pos == '-') {
        dice_binary_op_t op = (*state->pos == '+') ? DICE_OP_ADD : DICE_OP_SUB;
        state->pos++;
        
        dice_ast_node_t *operand = parse_unary(state);
        if (!operand) {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected expression after unary operator");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        if (op == DICE_OP_SUB) {
            // Create a unary minus as (0 - operand)
            dice_ast_node_t *zero = create_node(state->ctx, DICE_NODE_LITERAL);
            zero->data.literal.value = 0;
            
            dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_BINARY_OP);
            if (node) {
                node->data.binary_op.op = DICE_OP_SUB;
                node->data.binary_op.left = zero;
                node->data.binary_op.right = operand;
            }
            return node;
        } else {
            // Unary plus is just the operand
            return operand;
        }
    }
    
    return parse_primary(state);
}

static dice_ast_node_t* parse_product(parser_state_t *state) {
    dice_ast_node_t *left = parse_unary(state);
    if (!left) return NULL;
    
    while (1) {
        skip_whitespace(state);
        
        dice_binary_op_t op;
        if (*state->pos == '*') {
            op = DICE_OP_MUL;
        } else if (*state->pos == '/') {
            op = DICE_OP_DIV;
        } else {
            break;
        }
        
        state->pos++; // consume operator
        
        dice_ast_node_t *right = parse_unary(state);
        if (!right) {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected expression after operator");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_BINARY_OP);
        if (!node) return NULL;
        
        node->data.binary_op.op = op;
        node->data.binary_op.left = left;
        node->data.binary_op.right = right;
        left = node;
    }
    
    return left;
}

static dice_ast_node_t* parse_sum(parser_state_t *state) {
    dice_ast_node_t *left = parse_product(state);
    if (!left) return NULL;
    
    while (1) {
        skip_whitespace(state);
        
        dice_binary_op_t op;
        if (*state->pos == '+') {
            op = DICE_OP_ADD;
        } else if (*state->pos == '-') {
            op = DICE_OP_SUB;
        } else {
            break;
        }
        
        state->pos++; // consume operator
        
        dice_ast_node_t *right = parse_product(state);
        if (!right) {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected expression after operator");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_BINARY_OP);
        if (!node) return NULL;
        
        node->data.binary_op.op = op;
        node->data.binary_op.left = left;
        node->data.binary_op.right = right;
        left = node;
    }
    
    return left;
}

static dice_ast_node_t* parse_expression(parser_state_t *state) {
    return parse_sum(state);
}

dice_ast_node_t* dice_parse(dice_context_t *ctx, const char *expression_str) {
    if (!ctx || !expression_str) return NULL;
    
    parser_state_t state = {
        .ctx = ctx,
        .input = expression_str,
        .pos = expression_str
    };
    
    dice_ast_node_t *result = parse_expression(&state);
    
    if (result) {
        skip_whitespace(&state);
        if (*state.pos != '\0') {
            // Unexpected characters at end
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Unexpected characters at end of expression: '%s'", state.pos);
            ctx->error.has_error = true;
            return NULL;
        }
    }
    
    return result;
}

// =============================================================================
// Tracing Implementation
// =============================================================================

static void add_trace_entry(dice_context_t *ctx, dice_trace_entry_t *entry) {
    if (!ctx || !entry) return;
    
    entry->next = NULL;
    
    if (!ctx->trace.first) {
        ctx->trace.first = entry;
        ctx->trace.last = entry;
    } else {
        ctx->trace.last->next = entry;
        ctx->trace.last = entry;
    }
    
    ctx->trace.count++;
}

static void trace_atomic_roll(dice_context_t *ctx, int sides, int result) {
    dice_trace_entry_t *entry = arena_alloc(ctx, sizeof(dice_trace_entry_t));
    if (!entry) return;
    
    entry->type = TRACE_ATOMIC_ROLL;
    entry->data.atomic_roll.sides = sides;
    entry->data.atomic_roll.result = result;
    
    add_trace_entry(ctx, entry);
}

// =============================================================================
// Evaluator Implementation (Stateless)
// =============================================================================

dice_eval_result_t dice_evaluate(dice_context_t *ctx, const dice_ast_node_t *node) {
    dice_eval_result_t result = {0, false};
    
    if (!ctx || !node) {
        return result;
    }
    
    switch (node->type) {
        case DICE_NODE_LITERAL:
            result.value = node->data.literal.value;
            result.success = true;
            break;
            
        case DICE_NODE_BINARY_OP: {
            dice_eval_result_t left = dice_evaluate(ctx, node->data.binary_op.left);
            if (!left.success) return left;
            
            dice_eval_result_t right = dice_evaluate(ctx, node->data.binary_op.right);
            if (!right.success) return right;
            
            switch (node->data.binary_op.op) {
                case DICE_OP_ADD:
                    result.value = left.value + right.value;
                    break;
                case DICE_OP_SUB:
                    result.value = left.value - right.value;
                    break;
                case DICE_OP_MUL:
                    result.value = left.value * right.value;
                    break;
                case DICE_OP_DIV:
                    if (right.value == 0) {
                        snprintf(ctx->error.message, sizeof(ctx->error.message),
                                "Division by zero");
                        ctx->error.has_error = true;
                        return result;
                    }
                    result.value = left.value / right.value;
                    break;
            }
            result.success = true;
            break;
        }
        
        case DICE_NODE_DICE_OP: {
            // Evaluate count (default to 1)
            int64_t count = 1;
            if (node->data.dice_op.count) {
                dice_eval_result_t count_result = dice_evaluate(ctx, node->data.dice_op.count);
                if (!count_result.success) return count_result;
                count = count_result.value;
            }
            
            // Evaluate sides
            dice_eval_result_t sides_result = dice_evaluate(ctx, node->data.dice_op.sides);
            if (!sides_result.success) return sides_result;
            int64_t sides = sides_result.value;
            
            // Validate parameters
            if (count <= 0 || count > ctx->policy.max_dice_count) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Invalid dice count: %ld", count);
                ctx->error.has_error = true;
                return result;
            }
            
            if (sides <= 0 || sides > ctx->policy.max_sides) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Invalid dice sides: %ld", sides);
                ctx->error.has_error = true;
                return result;
            }
            
            // Roll dice based on type
            result.value = 0;
            for (int i = 0; i < count; i++) {
                int roll = ctx->rng.roll(ctx->rng.state, (int)sides);
                if (roll < 0) {
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "RNG error");
                    ctx->error.has_error = true;
                    return result;
                }
                
                trace_atomic_roll(ctx, (int)sides, roll);
                
                // Handle dice type modifiers
                switch (node->data.dice_op.dice_type) {
                    case DICE_DICE_BASIC:
                        result.value += roll;
                        break;
                        
                    case DICE_DICE_EXPLODING:
                        // Explode on maximum
                        while (roll == sides && ctx->policy.max_explosion_depth > 0) {
                            result.value += roll;
                            roll = ctx->rng.roll(ctx->rng.state, (int)sides);
                            trace_atomic_roll(ctx, (int)sides, roll);
                        }
                        result.value += roll;
                        break;
                        
                    default:
                        // Other dice types not implemented yet
                        result.value += roll;
                        break;
                }
            }
            result.success = true;
            break;
        }
        
        case DICE_NODE_FUNCTION_CALL:
            // Function calls not implemented yet
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Function calls not supported yet");
            ctx->error.has_error = true;
            break;
            
        case DICE_NODE_ANNOTATION:
            // Evaluate child and ignore annotation
            return dice_evaluate(ctx, node->data.annotation.child);
    }
    
    return result;
}

// =============================================================================
// RNG Implementations
// =============================================================================

// System RNG state
typedef struct {
    uint64_t seed;
} system_rng_state_t;

static int system_rng_init(void *state, uint64_t seed) {
    system_rng_state_t *s = (system_rng_state_t*)state;
    s->seed = seed ? seed : (uint64_t)time(NULL);
    srand((unsigned int)s->seed);
    return 0;
}

static int system_rng_roll(void *state, int sides) {
    (void)state; // unused
    if (sides <= 0) return -1;
    return (rand() % sides) + 1;
}

static uint64_t system_rng_rand(void *state, uint64_t max) {
    (void)state; // unused
    if (max == 0) return 0;
    return rand() % max;
}

static void system_rng_cleanup(void *state) {
    free(state);
}

dice_rng_vtable_t dice_create_system_rng(uint64_t seed) {
    system_rng_state_t *state = malloc(sizeof(system_rng_state_t));
    
    dice_rng_vtable_t rng = {
        .init = system_rng_init,
        .roll = system_rng_roll,
        .rand = system_rng_rand,
        .cleanup = system_rng_cleanup,
        .state = state
    };
    
    rng.init(state, seed);
    return rng;
}

// Placeholder for xoshiro - just use system RNG for now
dice_rng_vtable_t dice_create_xoshiro_rng(uint64_t seed) {
    return dice_create_system_rng(seed);
}

// =============================================================================
// Utility Functions
// =============================================================================

dice_policy_t dice_default_policy(void) {
    dice_policy_t policy = {
        .max_dice_count = 1000,
        .max_sides = 1000000,
        .max_explosion_depth = 10,
        .allow_negative_dice = false,
        .strict_mode = false
    };
    return policy;
}

dice_eval_result_t dice_roll_expression(dice_context_t *ctx, const char *expression_str) {
    dice_eval_result_t result = {0, false};
    
    if (!ctx || !expression_str) return result;
    
    dice_ast_node_t *ast = dice_parse(ctx, expression_str);
    if (!ast) return result;
    
    return dice_evaluate(ctx, ast);
}

// =============================================================================
// Error Handling and Tracing Accessors
// =============================================================================

bool dice_has_error(const dice_context_t *ctx) {
    return ctx ? ctx->error.has_error : true;
}

const char* dice_get_error(const dice_context_t *ctx) {
    return ctx ? ctx->error.message : "Invalid context";
}

void dice_clear_error(dice_context_t *ctx) {
    if (ctx) {
        ctx->error.has_error = false;
        ctx->error.code = 0;
        ctx->error.message[0] = '\0';
    }
}

const dice_trace_t* dice_get_trace(const dice_context_t *ctx) {
    return ctx ? &ctx->trace : NULL;
}

void dice_clear_trace(dice_context_t *ctx) {
    if (ctx) {
        memset(&ctx->trace, 0, sizeof(ctx->trace));
    }
}