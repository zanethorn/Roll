#include "dice.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
        // Initialize custom dice fields
        if (type == DICE_NODE_DICE_OP) {
            node->data.dice_op.custom_name = NULL;
            node->data.dice_op.custom_die = NULL;
            node->data.dice_op.selection = NULL;
        }
    }
    return node;
}

// Forward declarations
static dice_ast_node_t* parse_expression(parser_state_t *state);
static dice_ast_node_t* parse_sum(parser_state_t *state);
static dice_ast_node_t* parse_product(parser_state_t *state);
static dice_ast_node_t* parse_unary(parser_state_t *state);
static dice_ast_node_t* parse_primary(parser_state_t *state);
static dice_custom_die_t* parse_custom_die_definition(parser_state_t *state);

static dice_custom_die_t* parse_custom_die_definition(parser_state_t *state) {
    skip_whitespace(state);
    
    if (*state->pos != '{') {
        return NULL;
    }
    
    const char *start_pos = state->pos;
    state->pos++; // consume '{'
    skip_whitespace(state);
    
    // Count sides by parsing through the definition
    const char *temp_pos = state->pos;
    size_t side_count = 0;
    int brace_depth = 0;
    bool in_quotes = false;
    bool expecting_value = true;
    
    while (*temp_pos && (brace_depth > 0 || *temp_pos != '}')) {
        if (*temp_pos == '"' && (temp_pos == state->pos || *(temp_pos - 1) != '\\')) {
            in_quotes = !in_quotes;
        } else if (!in_quotes) {
            if (*temp_pos == '{') brace_depth++;
            else if (*temp_pos == '}') brace_depth--;
            else if (*temp_pos == ',' && brace_depth == 0 && expecting_value) {
                side_count++;
                expecting_value = true;
            } else if (!isspace(*temp_pos) && expecting_value) {
                expecting_value = false;
            }
        }
        temp_pos++;
    }
    
    if (expecting_value == false) side_count++; // Count the last item
    
    if (side_count == 0) {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Empty custom die definition");
        state->ctx->error.has_error = true;
        return NULL;
    }
    
    // Allocate custom die
    dice_custom_die_t *custom_die = arena_alloc(state->ctx, sizeof(dice_custom_die_t));
    if (!custom_die) return NULL;
    
    custom_die->sides = arena_alloc(state->ctx, side_count * sizeof(dice_custom_side_t));
    if (!custom_die->sides) return NULL;
    
    custom_die->name = NULL; // Inline die has no name
    custom_die->side_count = 0;
    
    // Parse sides
    while (*state->pos && *state->pos != '}') {
        skip_whitespace(state);
        
        if (*state->pos == '}') break;
        
        int64_t value = 0;
        const char *label = NULL;
        
        // Check if it's a number
        if (is_digit(*state->pos) || *state->pos == '-') {
            // Parse numeric value
            bool negative = false;
            if (*state->pos == '-') {
                negative = true;
                state->pos++;
            }
            
            while (is_digit(*state->pos)) {
                value = value * 10 + (*state->pos - '0');
                state->pos++;
            }
            
            if (negative) value = -value;
            
            skip_whitespace(state);
            
            // Check for label after colon
            if (*state->pos == ':') {
                state->pos++; // consume ':'
                skip_whitespace(state);
                
                if (*state->pos == '"') {
                    // Parse quoted string label
                    state->pos++; // consume opening quote
                    const char *start = state->pos;
                    while (*state->pos && *state->pos != '"') {
                        state->pos++;
                    }
                    if (*state->pos == '"') {
                        size_t len = state->pos - start;
                        char *label_copy = arena_alloc(state->ctx, len + 1);
                        if (label_copy) {
                            memcpy(label_copy, start, len);
                            label_copy[len] = '\0';
                            label = label_copy;
                        }
                        state->pos++; // consume closing quote
                    }
                }
            }
        } else if (*state->pos == '"') {
            // Quoted string without explicit value - use index as value
            value = custom_die->side_count; // 0-based indexing for implicit numbering
            
            state->pos++; // consume opening quote
            const char *start = state->pos;
            while (*state->pos && *state->pos != '"') {
                state->pos++;
            }
            if (*state->pos == '"') {
                size_t len = state->pos - start;
                char *label_copy = arena_alloc(state->ctx, len + 1);
                if (label_copy) {
                    memcpy(label_copy, start, len);
                    label_copy[len] = '\0';
                    label = label_copy;
                }
                state->pos++; // consume closing quote
            }
        } else {
            // Invalid syntax
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected number or quoted string in custom die definition");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        // Add the side
        if (custom_die->side_count < side_count) {
            custom_die->sides[custom_die->side_count].value = value;
            custom_die->sides[custom_die->side_count].label = label;
            custom_die->side_count++;
        }
        
        skip_whitespace(state);
        
        // Check for comma or end
        if (*state->pos == ',') {
            state->pos++; // consume comma
        } else if (*state->pos != '}') {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected ',' or '}' in custom die definition");
            state->ctx->error.has_error = true;
            return NULL;
        }
    }
    
    if (*state->pos != '}') {
        snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                "Expected closing '}' in custom die definition");
        state->ctx->error.has_error = true;
        return NULL;
    }
    
    state->pos++; // consume '}'
    return custom_die;
}

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
    
    skip_whitespace(state);
    
    dice_ast_node_t *node = create_node(state->ctx, DICE_NODE_DICE_OP);
    if (!node) return NULL;
    
    node->data.dice_op.count = count;
    node->data.dice_op.modifier = NULL;
    
    // Check for custom die syntax
    if (*state->pos == '{') {
        // Inline custom die definition: 1d{-1,0,1}
        dice_custom_die_t *custom_die = parse_custom_die_definition(state);
        if (!custom_die) return NULL;
        
        node->data.dice_op.dice_type = DICE_DICE_CUSTOM;
        node->data.dice_op.sides = NULL;
        node->data.dice_op.custom_die = custom_die;
        node->data.dice_op.custom_name = NULL;
        
    } else if (is_letter(*state->pos)) {
        // Named custom die: 1dF
        const char *name_start = state->pos;
        while (is_letter(*state->pos) || is_digit(*state->pos)) {
            state->pos++;
        }
        
        size_t name_len = state->pos - name_start;
        char *name = arena_alloc(state->ctx, name_len + 1);
        if (!name) return NULL;
        
        memcpy(name, name_start, name_len);
        name[name_len] = '\0';
        
        node->data.dice_op.dice_type = DICE_DICE_CUSTOM;
        node->data.dice_op.sides = NULL;
        node->data.dice_op.custom_die = NULL;
        node->data.dice_op.custom_name = name;
        
    } else {
        // Standard numeric sides
        dice_ast_node_t *sides = parse_number(state);
        if (!sides) {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected number of sides, custom die name, or custom die definition after 'd'");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        node->data.dice_op.dice_type = DICE_DICE_BASIC;
        node->data.dice_op.sides = sides;
    }
    
    // Check for keep/drop modifiers: kh, kl, dh, dl, k (shorthand for kh), d (shorthand for dl)
    skip_whitespace(state);
    if ((*state->pos == 'k' || *state->pos == 'K' || *state->pos == 'd' || *state->pos == 'D') &&
        ((*(state->pos + 1) == 'h' || *(state->pos + 1) == 'H' || 
          *(state->pos + 1) == 'l' || *(state->pos + 1) == 'L') ||
         (is_digit(*(state->pos + 1)) || *(state->pos + 1) == ' ' || *(state->pos + 1) == '\t'))) {
        
        char op1 = tolower(*state->pos);
        char op2 = '\0';
        
        // Check if it's a two-character operator (kh, kl, dh, dl) or single-character shorthand (k, d)
        if (*(state->pos + 1) == 'h' || *(state->pos + 1) == 'H' || 
            *(state->pos + 1) == 'l' || *(state->pos + 1) == 'L') {
            op2 = tolower(*(state->pos + 1));
        } else {
            // Single-character shorthand: 'k' defaults to 'kh', 'd' defaults to 'dl'
            op2 = (op1 == 'k') ? 'h' : 'l';
        }
        
        // Skip the operator (either 1 or 2 characters)
        if ((*(state->pos + 1) == 'h' || *(state->pos + 1) == 'H' || 
             *(state->pos + 1) == 'l' || *(state->pos + 1) == 'L')) {
            state->pos += 2; // Two-character operator
        } else {
            state->pos += 1; // Single-character shorthand
        }
        
        // Parse the count
        skip_whitespace(state);
        dice_ast_node_t *select_count = parse_number(state);
        if (!select_count) {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected number after %c%c modifier", op1, op2);
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        // Create selection structure
        dice_selection_t *selection = arena_alloc(state->ctx, sizeof(dice_selection_t));
        if (!selection) return NULL;
        
        // Initialize conditional selection fields
        selection->is_conditional = false;
        selection->comparison_op = DICE_OP_ADD; // Unused for non-conditional
        selection->comparison_value = 0; // Unused for non-conditional
        
        // Evaluate the selection count (for now, assume it's a literal)
        if (select_count->type == DICE_NODE_LITERAL) {
            selection->count = select_count->data.literal.value;
        } else {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Selection count must be a literal number");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        // Determine selection parameters and preserve original syntax
        char *syntax = arena_alloc(state->ctx, 3);
        if (!syntax) return NULL;
        
        // For shorthand syntax, preserve the original single character but use expanded logic
        if ((*(state->pos - 1) == 'h' || *(state->pos - 1) == 'H' || 
             *(state->pos - 1) == 'l' || *(state->pos - 1) == 'L')) {
            // Two-character syntax (kh, kl, dh, dl)
            syntax[0] = op1;
            syntax[1] = op2;
            syntax[2] = '\0';
        } else {
            // Single-character shorthand syntax (k, d)
            syntax[0] = op1;
            syntax[1] = '\0';
        }
        
        if (op1 == 'k') { // keep high/low
            selection->select_high = (op2 == 'h');
            selection->count = select_count->data.literal.value;
            selection->is_drop_operation = false;
            selection->original_syntax = syntax;
        } else { // drop high/low
            selection->select_high = (op2 == 'l'); // drop high = select low, drop low = select high
            selection->count = select_count->data.literal.value;
            selection->is_drop_operation = true;
            selection->original_syntax = syntax;
        }
        
        // Update the node to be a selection operation
        node->data.dice_op.dice_type = DICE_DICE_SELECT;
        node->data.dice_op.modifier = select_count;
        node->data.dice_op.selection = selection;
        
    } else if (*state->pos == 's' || *state->pos == 'S') {
        // Check for conditional selection: s>N, s<N, s>=N, s<=N, s==N, s!=N
        state->pos++; // consume 's'
        skip_whitespace(state);
        
        // Parse comparison operator
        dice_binary_op_t comp_op;
        if (*state->pos == '>' && *(state->pos + 1) == '=') {
            comp_op = DICE_OP_GTE;
            state->pos += 2;
        } else if (*state->pos == '<' && *(state->pos + 1) == '=') {
            comp_op = DICE_OP_LTE;
            state->pos += 2;
        } else if (*state->pos == '<' && *(state->pos + 1) == '>') {
            comp_op = DICE_OP_NEQ;
            state->pos += 2;
        } else if (*state->pos == '=') {
            comp_op = DICE_OP_EQ;
            state->pos++;
        } else if (*state->pos == '>') {
            comp_op = DICE_OP_GT;
            state->pos++;
        } else if (*state->pos == '<') {
            comp_op = DICE_OP_LT;
            state->pos++;
        } else {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected comparison operator after 's' (>, <, >=, <=, =, <>)");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        skip_whitespace(state);
        
        // Parse comparison value
        dice_ast_node_t *comp_value = parse_number(state);
        if (!comp_value || comp_value->type != DICE_NODE_LITERAL) {
            snprintf(state->ctx->error.message, sizeof(state->ctx->error.message),
                    "Expected numeric value after comparison operator");
            state->ctx->error.has_error = true;
            return NULL;
        }
        
        // Create conditional selection structure
        dice_selection_t *selection = arena_alloc(state->ctx, sizeof(dice_selection_t));
        if (!selection) return NULL;
        
        selection->is_conditional = true;
        selection->comparison_op = comp_op;
        selection->comparison_value = comp_value->data.literal.value;
        selection->count = 0; // Not used for conditional selection
        selection->select_high = false; // Not used for conditional selection
        selection->is_drop_operation = false; // Not used for conditional selection
        
        // Create original syntax string
        const char *op_str = "";
        switch (comp_op) {
            case DICE_OP_GT: op_str = ">"; break;
            case DICE_OP_LT: op_str = "<"; break;
            case DICE_OP_GTE: op_str = ">="; break;
            case DICE_OP_LTE: op_str = "<="; break;
            case DICE_OP_EQ: op_str = "="; break;
            case DICE_OP_NEQ: op_str = "<>"; break;
            default: op_str = "?"; break;
        }
        
        size_t syntax_len = snprintf(NULL, 0, "s%s%lld", op_str, (long long)comp_value->data.literal.value) + 1;
        char *syntax = arena_alloc(state->ctx, syntax_len);
        if (!syntax) return NULL;
        snprintf(syntax, syntax_len, "s%s%lld", op_str, (long long)comp_value->data.literal.value);
        selection->original_syntax = syntax;
        
        // Update the node to be a conditional selection operation
        node->data.dice_op.dice_type = DICE_DICE_CONDITIONAL_SELECT;
        node->data.dice_op.modifier = comp_value;
        node->data.dice_op.selection = selection;
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