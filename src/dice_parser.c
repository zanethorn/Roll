#include "dice_parser.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// AST node types
typedef enum {
    NODE_NUMBER,
    NODE_DICE,
    NODE_CUSTOM_DICE,
    NODE_BINARY_OP,
    NODE_UNARY_OP
} node_type_t;

// Binary operators
typedef enum {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV
} binary_op_t;

// Unary operators  
typedef enum {
    OP_PLUS,
    OP_MINUS
} unary_op_t;

// AST node structure
struct dice_expression {
    node_type_t type;
    union {
        struct {
            int value;
        } number;
        struct {
            dice_expression_t *count; // NULL for single die
            dice_expression_t *sides;
        } dice;
        struct {
            dice_expression_t *count; // NULL for single die
            char *identifier;
        } custom_dice;
        struct {
            binary_op_t op;
            dice_expression_t *left;
            dice_expression_t *right;
        } binary_op;
        struct {
            unary_op_t op;
            dice_expression_t *operand;
        } unary_op;
    } data;
};

// Parser state
typedef struct {
    const char *input;
    const char *pos;
    const char *error_msg;
} parser_state_t;

// Forward declarations
static dice_expression_t* parse_expression(parser_state_t *state);
static dice_expression_t* parse_sum(parser_state_t *state);
static dice_expression_t* parse_product(parser_state_t *state);
static dice_expression_t* parse_unary(parser_state_t *state);
static dice_expression_t* parse_primary(parser_state_t *state);
static dice_expression_t* parse_group(parser_state_t *state);
static dice_expression_t* parse_number(parser_state_t *state);
static dice_expression_t* parse_dice(parser_state_t *state);
static dice_expression_t* parse_custom_dice(parser_state_t *state);

// Default RNG implementation using system rand()
typedef struct {
    uint32_t seed;
} default_rng_state_t;

static void default_rng_init(void *state, uint32_t seed) {
    default_rng_state_t *s = (default_rng_state_t*)state;
    s->seed = seed ? seed : (uint32_t)time(NULL);
    srand(s->seed);
}

static int default_rng_roll(void *state, int sides) {
    (void)state; // Unused
    if (sides <= 0) return -1;
    return (rand() % sides) + 1;
}

static void default_rng_cleanup(void *state) {
    free(state);
}

// Utility functions
static void skip_whitespace(parser_state_t *state) {
    while (isspace(*state->pos)) {
        state->pos++;
    }
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_letter(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static dice_expression_t* create_node(node_type_t type) {
    dice_expression_t *node = malloc(sizeof(dice_expression_t));
    if (!node) return NULL;
    memset(node, 0, sizeof(dice_expression_t));
    node->type = type;
    return node;
}

// Parse number
static dice_expression_t* parse_number(parser_state_t *state) {
    skip_whitespace(state);
    
    if (!is_digit(*state->pos)) {
        return NULL;
    }
    
    int value = 0;
    while (is_digit(*state->pos)) {
        value = value * 10 + (*state->pos - '0');
        state->pos++;
    }
    
    dice_expression_t *node = create_node(NODE_NUMBER);
    if (node) {
        node->data.number.value = value;
    }
    return node;
}

// Parse identifier
static char* parse_identifier(parser_state_t *state) {
    skip_whitespace(state);
    
    if (!is_letter(*state->pos)) {
        return NULL;
    }
    
    const char *start = state->pos;
    while (is_letter(*state->pos) || is_digit(*state->pos) || *state->pos == '_') {
        state->pos++;
    }
    
    size_t len = state->pos - start;
    char *identifier = malloc(len + 1);
    if (identifier) {
        memcpy(identifier, start, len);
        identifier[len] = '\0';
    }
    return identifier;
}

// Parse dice expression (e.g., "3d6", "d20")
static dice_expression_t* parse_dice(parser_state_t *state) {
    skip_whitespace(state);
    
    dice_expression_t *count = NULL;
    
    // Check if there's a count before 'd'
    const char *saved_pos = state->pos;
    if (is_digit(*state->pos)) {
        count = parse_number(state);
        skip_whitespace(state);
        
        // If no 'd' follows, this isn't a dice expression
        if (*state->pos != 'd' && *state->pos != 'D') {
            dice_free_expression(count);
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
        dice_free_expression(count);
        return NULL;
    }
    
    skip_whitespace(state);
    
    // Check if this is custom dice (identifier) or regular dice (number)
    if (is_letter(*state->pos)) {
        // Custom dice
        char *identifier = parse_identifier(state);
        if (!identifier) {
            dice_free_expression(count);
            state->error_msg = "Expected identifier after 'd'";
            return NULL;
        }
        
        dice_expression_t *node = create_node(NODE_CUSTOM_DICE);
        if (node) {
            node->data.custom_dice.count = count;
            node->data.custom_dice.identifier = identifier;
        } else {
            dice_free_expression(count);
            free(identifier);
        }
        return node;
    } else {
        // Regular dice - parse sides  
        dice_expression_t *sides = parse_primary(state); // Changed from parse_expression
        if (!sides) {
            dice_free_expression(count);
            state->error_msg = "Expected number of sides after 'd'";
            return NULL;
        }
        
        dice_expression_t *node = create_node(NODE_DICE);
        if (node) {
            node->data.dice.count = count;
            node->data.dice.sides = sides;
        } else {
            dice_free_expression(count);
            dice_free_expression(sides);
        }
        return node;
    }
}

// Parse group (parenthesized expression)
static dice_expression_t* parse_group(parser_state_t *state) {
    skip_whitespace(state);
    
    if (*state->pos != '(') {
        return NULL;
    }
    
    state->pos++; // consume '('
    dice_expression_t *expr = parse_expression(state);
    if (!expr) {
        state->error_msg = "Expected expression after '('";
        return NULL;
    }
    
    skip_whitespace(state);
    if (*state->pos != ')') {
        dice_free_expression(expr);
        state->error_msg = "Expected ')' after expression";
        return NULL;
    }
    
    state->pos++; // consume ')'
    return expr;
}

// Parse primary expression
static dice_expression_t* parse_primary(parser_state_t *state) {
    skip_whitespace(state);
    
    dice_expression_t *result = NULL;
    
    // Try parsing in order: number, dice, group
    if (*state->pos == '(') {
        result = parse_group(state);
    } else if (is_digit(*state->pos) || *state->pos == 'd' || *state->pos == 'D') {
        // Try dice first, then number
        const char *saved_pos = state->pos;
        result = parse_dice(state);
        if (!result) {
            state->pos = saved_pos;
            result = parse_number(state);
        }
    }
    
    if (!result) {
        state->error_msg = "Expected number, dice expression, or parenthesized expression";
    }
    
    return result;
}

// Parse unary expression
static dice_expression_t* parse_unary(parser_state_t *state) {
    skip_whitespace(state);
    
    if (*state->pos == '+' || *state->pos == '-') {
        unary_op_t op = (*state->pos == '+') ? OP_PLUS : OP_MINUS;
        state->pos++;
        
        dice_expression_t *operand = parse_unary(state);
        if (!operand) {
            state->error_msg = "Expected expression after unary operator";
            return NULL;
        }
        
        dice_expression_t *node = create_node(NODE_UNARY_OP);
        if (node) {
            node->data.unary_op.op = op;
            node->data.unary_op.operand = operand;
        } else {
            dice_free_expression(operand);
        }
        return node;
    }
    
    return parse_primary(state);
}

// Parse product expression
static dice_expression_t* parse_product(parser_state_t *state) {
    dice_expression_t *left = parse_unary(state);
    if (!left) return NULL;
    
    while (1) {
        skip_whitespace(state);
        
        binary_op_t op;
        if (*state->pos == '*') {
            op = OP_MUL;
        } else if (*state->pos == '/') {
            op = OP_DIV;
        } else {
            break;
        }
        
        state->pos++; // consume operator
        
        dice_expression_t *right = parse_unary(state);
        if (!right) {
            dice_free_expression(left);
            state->error_msg = "Expected expression after operator";
            return NULL;
        }
        
        dice_expression_t *node = create_node(NODE_BINARY_OP);
        if (node) {
            node->data.binary_op.op = op;
            node->data.binary_op.left = left;
            node->data.binary_op.right = right;
            left = node;
        } else {
            dice_free_expression(left);
            dice_free_expression(right);
            return NULL;
        }
    }
    
    return left;
}

// Parse sum expression
static dice_expression_t* parse_sum(parser_state_t *state) {
    dice_expression_t *left = parse_product(state);
    if (!left) return NULL;
    
    while (1) {
        skip_whitespace(state);
        
        binary_op_t op;
        if (*state->pos == '+') {
            op = OP_ADD;
        } else if (*state->pos == '-') {
            op = OP_SUB;
        } else {
            break;
        }
        
        state->pos++; // consume operator
        
        dice_expression_t *right = parse_product(state);
        if (!right) {
            dice_free_expression(left);
            state->error_msg = "Expected expression after operator";
            return NULL;
        }
        
        dice_expression_t *node = create_node(NODE_BINARY_OP);
        if (node) {
            node->data.binary_op.op = op;
            node->data.binary_op.left = left;
            node->data.binary_op.right = right;
            left = node;
        } else {
            dice_free_expression(left);
            dice_free_expression(right);
            return NULL;
        }
    }
    
    return left;
}

// Parse expression (top level)
static dice_expression_t* parse_expression(parser_state_t *state) {
    return parse_sum(state);
}

// Public API implementation
dice_expression_t* dice_parse_expression(const char *expression_str) {
    if (!expression_str) return NULL;
    
    parser_state_t state = {
        .input = expression_str,
        .pos = expression_str,
        .error_msg = NULL
    };
    
    dice_expression_t *result = parse_expression(&state);
    
    if (result) {
        skip_whitespace(&state);
        if (*state.pos != '\0') {
            // Unexpected characters at end
            dice_free_expression(result);
            return NULL;
        }
    }
    
    return result;
}

// Evaluate expression recursively
dice_eval_result_t dice_evaluate_expression(const dice_expression_t *expr, dice_rng_t *rng) {
    dice_eval_result_t result = {0, 0, NULL};
    
    if (!expr || !rng) {
        result.error = 1;
        result.error_msg = "Invalid expression or RNG";
        return result;
    }
    
    switch (expr->type) {
        case NODE_NUMBER:
            result.value = expr->data.number.value;
            break;
            
        case NODE_DICE: {
            int count = 1;
            if (expr->data.dice.count) {
                dice_eval_result_t count_result = dice_evaluate_expression(expr->data.dice.count, rng);
                if (count_result.error) return count_result;
                count = count_result.value;
            }
            
            dice_eval_result_t sides_result = dice_evaluate_expression(expr->data.dice.sides, rng);
            if (sides_result.error) return sides_result;
            int sides = sides_result.value;
            
            if (count <= 0 || sides <= 0) {
                result.error = 1;
                result.error_msg = "Invalid dice count or sides";
                return result;
            }
            
            result.value = 0;
            for (int i = 0; i < count; i++) {
                int roll = rng->roll(rng->state, sides);
                if (roll < 0) {
                    result.error = 1;
                    result.error_msg = "RNG error";
                    return result;
                }
                result.value += roll;
            }
            break;
        }
        
        case NODE_CUSTOM_DICE:
            result.error = 1;
            result.error_msg = "Custom dice not supported yet";
            break;
            
        case NODE_BINARY_OP: {
            dice_eval_result_t left_result = dice_evaluate_expression(expr->data.binary_op.left, rng);
            if (left_result.error) return left_result;
            
            dice_eval_result_t right_result = dice_evaluate_expression(expr->data.binary_op.right, rng);
            if (right_result.error) return right_result;
            
            switch (expr->data.binary_op.op) {
                case OP_ADD:
                    result.value = left_result.value + right_result.value;
                    break;
                case OP_SUB:
                    result.value = left_result.value - right_result.value;
                    break;
                case OP_MUL:
                    result.value = left_result.value * right_result.value;
                    break;
                case OP_DIV:
                    if (right_result.value == 0) {
                        result.error = 1;
                        result.error_msg = "Division by zero";
                        return result;
                    }
                    result.value = left_result.value / right_result.value;
                    break;
            }
            break;
        }
        
        case NODE_UNARY_OP: {
            dice_eval_result_t operand_result = dice_evaluate_expression(expr->data.unary_op.operand, rng);
            if (operand_result.error) return operand_result;
            
            switch (expr->data.unary_op.op) {
                case OP_PLUS:
                    result.value = operand_result.value;
                    break;
                case OP_MINUS:
                    result.value = -operand_result.value;
                    break;
            }
            break;
        }
    }
    
    return result;
}

void dice_free_expression(dice_expression_t *expr) {
    if (!expr) return;
    
    switch (expr->type) {
        case NODE_DICE:
            dice_free_expression(expr->data.dice.count);
            dice_free_expression(expr->data.dice.sides);
            break;
        case NODE_CUSTOM_DICE:
            dice_free_expression(expr->data.custom_dice.count);
            free(expr->data.custom_dice.identifier);
            break;
        case NODE_BINARY_OP:
            dice_free_expression(expr->data.binary_op.left);
            dice_free_expression(expr->data.binary_op.right);
            break;
        case NODE_UNARY_OP:
            dice_free_expression(expr->data.unary_op.operand);
            break;
        case NODE_NUMBER:
            // Nothing to free
            break;
    }
    
    free(expr);
}

dice_rng_t* dice_create_default_rng(uint32_t seed) {
    dice_rng_t *rng = malloc(sizeof(dice_rng_t));
    if (!rng) return NULL;
    
    default_rng_state_t *state = malloc(sizeof(default_rng_state_t));
    if (!state) {
        free(rng);
        return NULL;
    }
    
    rng->init = default_rng_init;
    rng->roll = default_rng_roll;
    rng->state = state;
    rng->cleanup = default_rng_cleanup;
    
    rng->init(rng->state, seed);
    
    return rng;
}

void dice_free_rng(dice_rng_t *rng) {
    if (!rng) return;
    if (rng->cleanup) {
        rng->cleanup(rng->state);
    }
    free(rng);
}

dice_eval_result_t dice_parse_and_evaluate(const char *expression_str, dice_rng_t *rng) {
    dice_eval_result_t result = {0, 1, "Parse error"};
    
    dice_expression_t *expr = dice_parse_expression(expression_str);
    if (!expr) {
        return result;
    }
    
    result = dice_evaluate_expression(expr, rng);
    dice_free_expression(expr);
    
    return result;
}