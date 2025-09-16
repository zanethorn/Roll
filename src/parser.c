#include "dice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Internal functions for parser
void* arena_alloc(dice_context_t *ctx, size_t size);

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
    
    // Create dice node
    dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_DICE_OP);
    if (!node) return NULL;
    
    node->data.dice_op.dice_type = DICE_DICE_BASIC;
    node->data.dice_op.count = count;
    node->data.dice_op.sides = sides;
    node->data.dice_op.modifier = NULL;
    
    return node;
}

static dice_ast_node_t* parse_group(parser_state_t *state) {
    skip_whitespace(state);
    
    if (*state->pos != '(') {
        return NULL;
    }
    
    state->pos++; // consume '('
    dice_ast_node_t *expr = parse_expression(state);
    if (!expr) return NULL;
    
    skip_whitespace(state);
    if (*state->pos != ')') {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Expected closing parenthesis");
        state->ctx->error.has_error = true;
        return NULL;
    }
    
    state->pos++; // consume ')'
    return expr;
}

static dice_ast_node_t* parse_primary(parser_state_t *state) {
    skip_whitespace(state);
    
    dice_ast_node_t *result = NULL;
    
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
        if (!operand) return NULL;
        
        if (op == DICE_OP_ADD) {
            // Unary plus is identity
            return operand;
        } else {
            // Unary minus: create 0 - operand
            dice_ast_node_t *zero = create_node(state->ctx, DICE_NODE_LITERAL);
            if (!zero) return NULL;
            zero->data.literal.value = 0;
            
            dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_BINARY_OP);
            if (!node) return NULL;
            
            node->data.binary_op.op = DICE_OP_SUB;
            node->data.binary_op.left = zero;
            node->data.binary_op.right = operand;
            return node;
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