#include "test_common.h"

// =============================================================================
// Dice Selection Tests (Keep/Drop High/Low Operations)
// =============================================================================

int test_basic_selection_operations() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test basic keep high operation
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6kh3");
    TEST_ASSERT(result.success, "Keep high operation succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep high result in valid range");
    
    // Test basic keep low operation
    result = dice_roll_expression(ctx, "4d6kl2");
    TEST_ASSERT(result.success, "Keep low operation succeeds");
    TEST_ASSERT(result.value >= 2 && result.value <= 12, "Keep low result in valid range");
    
    // Test basic drop high operation
    result = dice_roll_expression(ctx, "5d6dh2");
    TEST_ASSERT(result.success, "Drop high operation succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Drop high result in valid range");
    
    // Test basic drop low operation
    result = dice_roll_expression(ctx, "5d6dl2");
    TEST_ASSERT(result.success, "Drop low operation succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Drop low result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_case_insensitivity() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test uppercase variants
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6KH3");
    TEST_ASSERT(result.success, "Uppercase KH works");
    
    result = dice_roll_expression(ctx, "4d6KL2");
    TEST_ASSERT(result.success, "Uppercase KL works");
    
    result = dice_roll_expression(ctx, "4d6DH1");
    TEST_ASSERT(result.success, "Uppercase DH works");
    
    result = dice_roll_expression(ctx, "4d6DL1");
    TEST_ASSERT(result.success, "Uppercase DL works");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_equivalence() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test equivalence with fixed seed
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    // Test: 4d6kh3 should equal 4d6dl1
    dice_eval_result_t result1 = dice_roll_expression(ctx, "4d6kh3");
    TEST_ASSERT(result1.success, "4d6kh3 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(12345);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result2 = dice_roll_expression(ctx, "4d6dl1");
    TEST_ASSERT(result2.success, "4d6dl1 succeeds");
    TEST_ASSERT(result1.value == result2.value, "4d6kh3 equals 4d6dl1 with same seed");
    
    // Test: 5d6kl2 should equal 5d6dh3
    dice_context_reset(ctx);
    rng = dice_create_system_rng(54321);
    dice_context_set_rng(ctx, &rng);
    
    result1 = dice_roll_expression(ctx, "5d6kl2");
    TEST_ASSERT(result1.success, "5d6kl2 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(54321);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    result2 = dice_roll_expression(ctx, "5d6dh3");
    TEST_ASSERT(result2.success, "5d6dh3 succeeds");
    TEST_ASSERT(result1.value == result2.value, "5d6kl2 equals 5d6dh3 with same seed");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_error_handling() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test keeping more dice than available
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6kh5");
    TEST_ASSERT(!result.success, "Cannot keep more dice than rolled");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set for invalid keep");
    
    dice_context_reset(ctx);
    
    // Test dropping more dice than available
    result = dice_roll_expression(ctx, "3d6dl4");
    TEST_ASSERT(!result.success, "Cannot drop more dice than rolled");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set for invalid drop");
    
    dice_context_reset(ctx);
    
    // Test dropping all dice
    result = dice_roll_expression(ctx, "3d6dl3");
    TEST_ASSERT(!result.success, "Cannot drop all dice");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set for dropping all dice");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_edge_cases() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test keeping all dice (should work like normal roll)
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6kh3");
    TEST_ASSERT(result.success, "Keeping all dice works");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep all result in valid range");
    
    // Test dropping zero dice (should work like normal roll)
    result = dice_roll_expression(ctx, "3d6dl0");
    TEST_ASSERT(result.success, "Dropping zero dice works");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Drop zero result in valid range");
    
    // Test single die operations
    result = dice_roll_expression(ctx, "1d20kh1");
    TEST_ASSERT(result.success, "Single die keep high works");
    TEST_ASSERT(result.value >= 1 && result.value <= 20, "Single die keep result in valid range");
    
    result = dice_roll_expression(ctx, "1d20dl0");
    TEST_ASSERT(result.success, "Single die drop zero works");
    TEST_ASSERT(result.value >= 1 && result.value <= 20, "Single die drop zero result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_in_complex_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test selection in arithmetic expressions
    dice_eval_result_t result = dice_roll_expression(ctx, "1d20+4d6kh3");
    TEST_ASSERT(result.success, "Selection in addition works");
    TEST_ASSERT(result.value >= 4 && result.value <= 38, "Complex expression result in valid range");
    
    // Test multiple selections
    result = dice_roll_expression(ctx, "4d6kh3+3d8dl1");
    TEST_ASSERT(result.success, "Multiple selections work");
    TEST_ASSERT(result.value >= 5 && result.value <= 34, "Multiple selections result in valid range");
    
    // Test with parentheses
    result = dice_roll_expression(ctx, "(4d6kh3)*2");
    TEST_ASSERT(result.success, "Selection with parentheses works");
    TEST_ASSERT(result.value >= 6 && result.value <= 36, "Parenthesized selection result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Test Runner
// =============================================================================

int main() {
    printf("Running dice selection tests...\\n\\n");
    
    RUN_TEST(test_basic_selection_operations);
    RUN_TEST(test_selection_case_insensitivity);
    RUN_TEST(test_selection_equivalence);
    RUN_TEST(test_selection_error_handling);
    RUN_TEST(test_selection_edge_cases);
    RUN_TEST(test_selection_in_complex_expressions);
    
    printf("All dice selection tests passed!\\n");
    return 0;
}