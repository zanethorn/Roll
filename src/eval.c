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
                
                if (node->data.dice_op.dice_type == DICE_DICE_FILTER) {
                    // Handle filter operations (kh/kl/dh/dl/s>N/s<N unified)
                    sum = evaluate_dice_filter(ctx, count, (int)sides, node->data.dice_op.selection);
                    if (ctx->error.has_error) {
                        return result; // Error was set by evaluate_dice_filter
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

int64_t evaluate_dice_filter(dice_context_t *ctx, int64_t count, int sides, const dice_selection_t *selection) {
    if (!ctx || !selection) return 0;
    
    // Allocate array to store all roll results
    int *rolls = arena_alloc(ctx, count * sizeof(int));
    if (!rolls) {
        snprintf(ctx->error.message, sizeof(ctx->error.message),
                "Failed to allocate memory for dice rolls");
        ctx->error.has_error = true;
        return 0;
    }
    
    // Allocate array to track which dice are selected
    bool *selected = arena_alloc(ctx, count * sizeof(bool));
    if (!selected) {
        snprintf(ctx->error.message, sizeof(ctx->error.message),
                "Failed to allocate memory for selection tracking");
        ctx->error.has_error = true;
        return 0;
    }
    
    // Initialize selection array
    for (int i = 0; i < count; i++) {
        selected[i] = false;
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
    }
    
    int64_t sum = 0;
    
    if (selection->is_conditional && selection->is_reroll) {
        // Reroll operations (r, r1, r>N, r<N, etc.)
        for (int i = 0; i < count; i++) {
            bool should_reroll = true;
            int roll = rolls[i];
            int reroll_count = 0;
            const int max_rerolls = 100; // Safety limit to prevent infinite loops
            
            while (should_reroll && reroll_count < max_rerolls) {
                int64_t comp_value = selection->comparison_value;
                bool matches_reroll_condition = false;
                
                switch (selection->comparison_op) {
                    case DICE_OP_GT:
                        matches_reroll_condition = (roll > comp_value);
                        break;
                    case DICE_OP_LT:
                        matches_reroll_condition = (roll < comp_value);
                        break;
                    case DICE_OP_GTE:
                        matches_reroll_condition = (roll >= comp_value);
                        break;
                    case DICE_OP_LTE:
                        matches_reroll_condition = (roll <= comp_value);
                        break;
                    case DICE_OP_EQ:
                        matches_reroll_condition = (roll == comp_value);
                        break;
                    case DICE_OP_NEQ:
                        matches_reroll_condition = (roll != comp_value);
                        break;
                    default:
                        snprintf(ctx->error.message, sizeof(ctx->error.message),
                                "Unknown comparison operator in reroll operation");
                        ctx->error.has_error = true;
                        return 0;
                }
                
                if (matches_reroll_condition) {
                    // Mark as rerolled for tracing
                    trace_atomic_roll_selected(ctx, sides, roll, false);
                    
                    // Reroll the die
                    int new_roll = ctx->rng.roll(ctx->rng.state, sides);
                    if (new_roll < 0) {
                        snprintf(ctx->error.message, sizeof(ctx->error.message),
                                "RNG error during reroll");
                        ctx->error.has_error = true;
                        return 0;
                    }
                    roll = new_roll;
                    reroll_count++;
                } else {
                    should_reroll = false;
                }
            }
            
            if (reroll_count >= max_rerolls) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Maximum reroll limit (%d) exceeded for die %d", max_rerolls, i + 1);
                ctx->error.has_error = true;
                return 0;
            }
            
            // Store final result
            rolls[i] = roll;
            selected[i] = true; // All dice are selected in reroll operations
            sum += roll;
        }
    } else if (selection->is_conditional) {
        // Conditional filtering (s>N, s<N, etc.)
        for (int i = 0; i < count; i++) {
            bool matches = false;
            int roll = rolls[i];
            int64_t comp_value = selection->comparison_value;
            
            switch (selection->comparison_op) {
                case DICE_OP_GT:
                    matches = (roll > comp_value);
                    break;
                case DICE_OP_LT:
                    matches = (roll < comp_value);
                    break;
                case DICE_OP_GTE:
                    matches = (roll >= comp_value);
                    break;
                case DICE_OP_LTE:
                    matches = (roll <= comp_value);
                    break;
                case DICE_OP_EQ:
                    matches = (roll == comp_value);
                    break;
                case DICE_OP_NEQ:
                    matches = (roll != comp_value);
                    break;
                default:
                    snprintf(ctx->error.message, sizeof(ctx->error.message),
                            "Unknown comparison operator in conditional filter");
                    ctx->error.has_error = true;
                    return 0;
            }
            
            if (matches) {
                selected[i] = true;
                sum += roll;
            }
        }
    } else {
        // Count-based filtering (keep/drop operations)
        // Calculate actual selection count
        int64_t actual_select_count;
        if (selection->is_drop_operation) {
            // Drop operation - convert to select count
            int64_t drop_count = selection->count;
            actual_select_count = count - drop_count;
            
            if (drop_count >= count) {
                // Dropping more dice than available - result is 0 (all dice dropped)
                actual_select_count = 0;
            }
        } else {
            // Keep operation
            actual_select_count = selection->count;
            
            if (actual_select_count > count) {
                // Keeping more dice than available - keep all dice that were rolled
                actual_select_count = count;
            }
        }
        
        // Validate selection count
        if (actual_select_count < 0) {
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Invalid selection count: %lld (must be non-negative)", 
                    (long long)actual_select_count);
            ctx->error.has_error = true;
            return 0;
        }
        
        // Special case: if selecting 0 dice (drop all or more), return 0
        if (actual_select_count == 0) {
            return 0;
        }
        
        // We need to track original indices to mark the correct dice as selected
        // Create an array of indices paired with values for sorting
        typedef struct {
            int value;
            int original_index;
        } roll_with_index_t;
        
        roll_with_index_t *indexed_rolls = arena_alloc(ctx, count * sizeof(roll_with_index_t));
        if (!indexed_rolls) {
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Failed to allocate memory for indexed rolls");
            ctx->error.has_error = true;
            return 0;
        }
        
        // Fill the indexed array
        for (int i = 0; i < count; i++) {
            indexed_rolls[i].value = rolls[i];
            indexed_rolls[i].original_index = i;
        }
        
        // Sort indexed rolls based on selection type
        if (selection->select_high) {
            // Sort high to low - we want the highest values first
            for (int i = 0; i < count - 1; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (indexed_rolls[j].value > indexed_rolls[i].value) {
                        roll_with_index_t temp = indexed_rolls[i];
                        indexed_rolls[i] = indexed_rolls[j];
                        indexed_rolls[j] = temp;
                    }
                }
            }
        } else {
            // Sort low to high - we want the lowest values first
            for (int i = 0; i < count - 1; i++) {
                for (int j = i + 1; j < count; j++) {
                    if (indexed_rolls[j].value < indexed_rolls[i].value) {
                        roll_with_index_t temp = indexed_rolls[i];
                        indexed_rolls[i] = indexed_rolls[j];
                        indexed_rolls[j] = temp;
                    }
                }
            }
        }
        
        // Mark the selected dice and sum them
        for (int i = 0; i < actual_select_count; i++) {
            int original_idx = indexed_rolls[i].original_index;
            selected[original_idx] = true;
            sum += indexed_rolls[i].value;
        }
    }
    
    // Now add all dice to trace with their selection status
    for (int i = 0; i < count; i++) {
        trace_atomic_roll_selected(ctx, sides, rolls[i], selected[i]);
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