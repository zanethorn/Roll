#include "test_common.h"

// =============================================================================
// Conditional Selection Tests (s>N, s<N, s>=N, s<=N, s==N, s!=N)
// =============================================================================

int test_basic_conditional_selection() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test greater than
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6s>2");
    TEST_ASSERT(result.success, "Conditional selection s>2 succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 18, "s>2 result in valid range");
    
    // Test less than
    result = dice_roll_expression(ctx, "4d6s<4");
    TEST_ASSERT(result.success, "Conditional selection s<4 succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 24, "s<4 result in valid range");
    
    // Test greater than or equal
    result = dice_roll_expression(ctx, "5d6s>=3");
    TEST_ASSERT(result.success, "Conditional selection s>=3 succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 30, "s>=3 result in valid range");
    
    // Test less than or equal
    result = dice_roll_expression(ctx, "3d6s<=5");
    TEST_ASSERT(result.success, "Conditional selection s<=5 succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 18, "s<=5 result in valid range");
    
    // Test equality
    result = dice_roll_expression(ctx, "10d6s==6");
    TEST_ASSERT(result.success, "Conditional selection s==6 succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 60, "s==6 result in valid range");
    
    // Test inequality
    result = dice_roll_expression(ctx, "4d6s!=1");
    TEST_ASSERT(result.success, "Conditional selection s!=1 succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 24, "s!=1 result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_edge_cases() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set deterministic seed for predictable results
    dice_rng_vtable_t rng = dice_create_system_rng(42);
    dice_context_set_rng(ctx, &rng);
    
    // Test condition that matches all dice (s>0 on d6)
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6s>0");
    TEST_ASSERT(result.success, "Condition matching all dice succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "All dice matched result in valid range");
    
    // Reset seed for next test
    rng = dice_create_system_rng(42);
    dice_context_set_rng(ctx, &rng);
    
    // Test condition that matches no dice (s>6 on d6)
    result = dice_roll_expression(ctx, "3d6s>6");
    TEST_ASSERT(result.success, "Condition matching no dice succeeds");
    TEST_ASSERT(result.value == 0, "No dice matched should give 0");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_case_insensitivity() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test uppercase 'S'
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6S>3");
    TEST_ASSERT(result.success, "Uppercase S operator works");
    TEST_ASSERT(result.value >= 0 && result.value <= 18, "Uppercase S result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_in_complex_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test conditional selection in addition
    dice_eval_result_t result = dice_roll_expression(ctx, "1d20+4d6s>3");
    TEST_ASSERT(result.success, "Conditional selection in addition works");
    TEST_ASSERT(result.value >= 1 && result.value <= 44, "Complex expression result in valid range");
    
    // Test multiple conditional selections
    result = dice_roll_expression(ctx, "3d6s>3+2d8s<=4");
    TEST_ASSERT(result.success, "Multiple conditional selections work");
    TEST_ASSERT(result.value >= 0 && result.value <= 26, "Multiple conditional selections result in valid range");
    
    // Test with parentheses
    result = dice_roll_expression(ctx, "(4d6s>=3)*2");
    TEST_ASSERT(result.success, "Conditional selection with parentheses works");
    TEST_ASSERT(result.value >= 0 && result.value <= 48, "Parenthesized conditional selection result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_with_different_dice() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test with d4
    dice_eval_result_t result = dice_roll_expression(ctx, "5d4s>2");
    TEST_ASSERT(result.success, "Conditional selection on d4 works");
    TEST_ASSERT(result.value >= 0 && result.value <= 20, "d4 conditional selection result in valid range");
    
    // Test with d20
    result = dice_roll_expression(ctx, "2d20s>=15");
    TEST_ASSERT(result.success, "Conditional selection on d20 works");
    TEST_ASSERT(result.value >= 0 && result.value <= 40, "d20 conditional selection result in valid range");
    
    // Test with d100
    result = dice_roll_expression(ctx, "3d100s<50");
    TEST_ASSERT(result.success, "Conditional selection on d100 works");
    TEST_ASSERT(result.value >= 0 && result.value <= 300, "d100 conditional selection result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_deterministic() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test with deterministic seed to verify logic
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    // Roll the dice to see what we get first
    dice_eval_result_t baseline = dice_roll_expression(ctx, "3d6");
    TEST_ASSERT(baseline.success, "Baseline roll succeeds");
    
    // Reset with same seed to test conditional logic
    rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t all_dice = dice_roll_expression(ctx, "3d6s>=1");
    TEST_ASSERT(all_dice.success, "All dice selection succeeds");
    TEST_ASSERT(all_dice.value == baseline.value, "s>=1 should equal sum of all dice");
    
    // Reset and test no dice
    rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t no_dice = dice_roll_expression(ctx, "3d6s>6");
    TEST_ASSERT(no_dice.success, "No dice selection succeeds");
    TEST_ASSERT(no_dice.value == 0, "s>6 on d6 should give 0");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_error_handling() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test invalid comparison operator
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6s@3");
    TEST_ASSERT(!result.success, "Invalid comparison operator should fail");
    TEST_ASSERT(ctx->error.has_error, "Error flag should be set for invalid operator");
    
    // Clear error for next test
    ctx->error.has_error = false;
    
    // Test missing comparison value
    result = dice_roll_expression(ctx, "3d6s>");
    TEST_ASSERT(!result.success, "Missing comparison value should fail");
    TEST_ASSERT(ctx->error.has_error, "Error flag should be set for missing value");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Test Runner
// =============================================================================

int main() {
    printf("Running conditional selection tests...\\n\\n");
    
    RUN_TEST(test_basic_conditional_selection);
    RUN_TEST(test_conditional_selection_edge_cases);
    RUN_TEST(test_conditional_selection_case_insensitivity);
    RUN_TEST(test_conditional_selection_in_complex_expressions);
    RUN_TEST(test_conditional_selection_with_different_dice);
    RUN_TEST(test_conditional_selection_deterministic);
    RUN_TEST(test_conditional_selection_error_handling);
    
    printf("All conditional selection tests passed!\\n");
    return 0;
}