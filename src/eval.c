#include "dice.h"
#include "internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
                default:
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "Unknown binary operator");
                    ctx->error.has_error = true;
                    return result;
            }
            result.success = true;
            break;
        }
        
        case DICE_NODE_DICE_OP: {
            // Evaluate count
            int64_t count = 1;
            if (node->data.dice_op.count) {
                dice_eval_result_t count_result = dice_evaluate(ctx, node->data.dice_op.count);
                if (!count_result.success) return count_result;
                count = count_result.value;
            }
            
            // Validation
            if (count <= 0) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Dice count must be positive, got %lld", (long long)count);
                ctx->error.has_error = true;
                return result;
            }
            
            if (count > ctx->policy.max_dice_count) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Too many dice: %lld exceeds limit of %d", 
                        (long long)count, ctx->policy.max_dice_count);
                ctx->error.has_error = true;
                return result;
            }
            
            int64_t sum = 0;
            
            if (node->data.dice_op.dice_type == DICE_DICE_CUSTOM) {
                // Handle custom dice
                const dice_custom_die_t *custom_die = NULL;
                
                if (node->data.dice_op.custom_die) {
                    // Inline custom die
                    custom_die = node->data.dice_op.custom_die;
                } else if (node->data.dice_op.custom_name) {
                    // Named custom die - look up in registry
                    custom_die = dice_lookup_custom_die(ctx, node->data.dice_op.custom_name);
                    if (!custom_die) {
                        snprintf(ctx->error.message, sizeof(ctx->error.message),
                                "Unknown custom die: %s", node->data.dice_op.custom_name);
                        ctx->error.has_error = true;
                        return result;
                    }
                } else {
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "Custom die has no definition or name");
                    ctx->error.has_error = true;
                    return result;
                }
                
                if (custom_die->side_count == 0) {
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "Custom die has no sides");
                    ctx->error.has_error = true;
                    return result;
                }
                
                // Roll custom dice
                for (int i = 0; i < count; i++) {
                    // Generate random index for the side
                    uint64_t rand_val = ctx->rng.rand(ctx->rng.state, custom_die->side_count);
                    if (rand_val >= custom_die->side_count) {
                        // Fallback to simple modulo if rand function misbehaves
                        rand_val = rand_val % custom_die->side_count;
                    }
                    
                    int64_t roll_value = custom_die->sides[rand_val].value;
                    
                    // Add to trace (use side count as "sides" for tracing purposes)
                    trace_atomic_roll(ctx, (int)custom_die->side_count, (int)roll_value);
                    
                    sum += roll_value;
                }
                
            } else {
                // Handle standard dice (basic or with selection)
                dice_eval_result_t sides_result = dice_evaluate(ctx, node->data.dice_op.sides);
                if (!sides_result.success) return sides_result;
                int64_t sides = sides_result.value;
                
                if (sides <= 0) {
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "Dice sides must be positive, got %lld", (long long)sides);
                    ctx->error.has_error = true;
                    return result;
                }
                
                if (sides > ctx->policy.max_sides) {
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "Too many sides: %lld exceeds limit of %d", 
                            (long long)sides, ctx->policy.max_sides);
                    ctx->error.has_error = true;
                    return result;
                }
                
                if (node->data.dice_op.dice_type == DICE_DICE_SELECT) {
                    // Handle selection operations (kh/kl/dh/dl)
                    sum = evaluate_dice_selection(ctx, count, (int)sides, node->data.dice_op.selection);
                    if (ctx->error.has_error) {
                        return result; // Error was set by evaluate_dice_selection
                    }
                } else {
                    // Roll standard dice (basic operation)
                    for (int i = 0; i < count; i++) {
                        int roll = ctx->rng.roll(ctx->rng.state, (int)sides);
                        if (roll < 0) {
                            snprintf(ctx->error.message, sizeof(ctx->error.message),
                                    "RNG error during dice roll");
                            ctx->error.has_error = true;
                            return result;
                        }
                        
                        // Add to trace
                        trace_atomic_roll(ctx, (int)sides, roll);
                        
                        sum += roll;
                    }
                }
            }
            
            result.value = sum;
            result.success = true;
            break;
        }
        
        case DICE_NODE_FUNCTION_CALL:
            // Function calls not yet implemented
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Function calls not yet supported: %s", 
                    node->data.function_call.name);
            ctx->error.has_error = true;
            break;
            
        case DICE_NODE_ANNOTATION:
            // Evaluate child and ignore annotation
            return dice_evaluate(ctx, node->data.annotation.child);
    }
    
    return result;
}

// Comparison function for sorting dice rolls
static int compare_rolls_desc(const void *a, const void *b) {
    int roll_a = *(const int*)a;
    int roll_b = *(const int*)b;
    return roll_b - roll_a; // Descending order
}

static int compare_rolls_asc(const void *a, const void *b) {
    int roll_a = *(const int*)a;
    int roll_b = *(const int*)b;
    return roll_a - roll_b; // Ascending order
}

int64_t evaluate_dice_selection(dice_context_t *ctx, int64_t count, int sides, const dice_selection_t *selection) {
    if (!ctx || !selection) return 0;
    
    // Calculate actual selection count
    int64_t actual_select_count;
    if (selection->is_drop_operation) {
        // Drop operation - convert to select count
        int64_t drop_count = selection->count;
        actual_select_count = count - drop_count;
        
        if (drop_count >= count) {
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Cannot drop %lld dice from only %lld dice", 
                    (long long)drop_count, (long long)count);
            ctx->error.has_error = true;
            return 0;
        }
    } else {
        // Keep operation
        actual_select_count = selection->count;
        
        if (actual_select_count > count) {
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Cannot keep %lld dice from only %lld dice", 
                    (long long)actual_select_count, (long long)count);
            ctx->error.has_error = true;
            return 0;
        }
    }
    
    // Validate selection count
    if (actual_select_count <= 0) {
        snprintf(ctx->error.message, sizeof(ctx->error.message),
                "Invalid selection count: %lld (must be positive)", 
                (long long)actual_select_count);
        ctx->error.has_error = true;
        return 0;
    }
    
    // Allocate array to store all roll results
    int *rolls = arena_alloc(ctx, count * sizeof(int));
    if (!rolls) {
        snprintf(ctx->error.message, sizeof(ctx->error.message),
                "Failed to allocate memory for dice rolls");
        ctx->error.has_error = true;
        return 0;
    }
    
    // Roll all dice and store results
    for (int i = 0; i < count; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, sides);
        if (roll < 0) {
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "RNG error during dice roll");
            ctx->error.has_error = true;
            return 0;
        }
        rolls[i] = roll;
        
        // Add to trace with selection annotation
        trace_atomic_roll(ctx, sides, roll);
    }
    
    // Sort rolls based on selection type
    if (selection->select_high) {
        qsort(rolls, count, sizeof(int), compare_rolls_desc); // High to low
    } else {
        qsort(rolls, count, sizeof(int), compare_rolls_asc);  // Low to high
    }
    
    // Sum the selected dice
    int64_t sum = 0;
    for (int i = 0; i < actual_select_count; i++) {
        sum += rolls[i];
    }
    
    return sum;
}

dice_eval_result_t dice_roll_expression(dice_context_t *ctx, const char *expression_str) {
    dice_eval_result_t result = {0, false};
    
    if (!ctx || !expression_str) return result;
    
    dice_ast_node_t *ast = dice_parse(ctx, expression_str);
    if (!ast) return result;
    
    return dice_evaluate(ctx, ast);
}