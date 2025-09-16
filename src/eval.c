#include "dice.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// External functions
void trace_atomic_roll(dice_context_t *ctx, int sides, int result);

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
            // Evaluate count and sides
            int64_t count = 1;
            if (node->data.dice_op.count) {
                dice_eval_result_t count_result = dice_evaluate(ctx, node->data.dice_op.count);
                if (!count_result.success) return count_result;
                count = count_result.value;
            }
            
            dice_eval_result_t sides_result = dice_evaluate(ctx, node->data.dice_op.sides);
            if (!sides_result.success) return sides_result;
            int64_t sides = sides_result.value;
            
            // Validation
            if (count <= 0) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Dice count must be positive, got %lld", (long long)count);
                ctx->error.has_error = true;
                return result;
            }
            
            if (sides <= 0) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Dice sides must be positive, got %lld", (long long)sides);
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
            
            if (sides > ctx->policy.max_sides) {
                snprintf(ctx->error.message, sizeof(ctx->error.message),
                        "Too many sides: %lld exceeds limit of %d", 
                        (long long)sides, ctx->policy.max_sides);
                ctx->error.has_error = true;
                return result;
            }
            
            // Roll dice
            int64_t sum = 0;
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

dice_eval_result_t dice_roll_expression(dice_context_t *ctx, const char *expression_str) {
    dice_eval_result_t result = {0, false};
    
    if (!ctx || !expression_str) return result;
    
    dice_ast_node_t *ast = dice_parse(ctx, expression_str);
    if (!ast) return result;
    
    return dice_evaluate(ctx, ast);
}