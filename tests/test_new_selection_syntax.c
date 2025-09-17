#include "test_common.h"

// =============================================================================
// New Selection Syntax Comprehensive Tests
// =============================================================================

int test_new_syntax_requirements() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test requirement 1: 'l' character for drop selector
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6l1");
    TEST_ASSERT(result.success, "New 'l' drop syntax works");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Drop 1 low result in valid range");
    
    // Test requirement 2: 'k' character for keep
    result = dice_roll_expression(ctx, "4d6k3");
    TEST_ASSERT(result.success, "Keep 'k' syntax works");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep 3 high result in valid range");
    
    // Test requirement 3: 'h' as substitute for 'k'
    result = dice_roll_expression(ctx, "4d6h3");
    TEST_ASSERT(result.success, "Keep 'h' alias syntax works");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep 3 high with h result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_old_syntax_rejection() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test that old 'd' for drop is rejected
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6d1");
    TEST_ASSERT(!result.success, "Old 'd' drop syntax should be rejected");
    
    // Test that two-character combinations are rejected
    result = dice_roll_expression(ctx, "4d6kh3");
    TEST_ASSERT(!result.success, "Two-character 'kh' should be rejected");
    
    result = dice_roll_expression(ctx, "4d6kl2");
    TEST_ASSERT(!result.success, "Two-character 'kl' should be rejected");
    
    result = dice_roll_expression(ctx, "4d6dh1");
    TEST_ASSERT(!result.success, "Two-character 'dh' should be rejected");
    
    result = dice_roll_expression(ctx, "4d6dl2");
    TEST_ASSERT(!result.success, "Two-character 'dl' should be rejected");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_default_values() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test: 4d6k = 4d6k1 (default to 1)
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6k");
    TEST_ASSERT(result.success, "Keep with no value defaults to 1");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "Default keep result in valid range");
    
    // Test: 3d8l = 3d8l1 (default to 1)
    result = dice_roll_expression(ctx, "3d8l");
    TEST_ASSERT(result.success, "Drop with no value defaults to 1");
    TEST_ASSERT(result.value >= 2 && result.value <= 16, "Default drop result in valid range");
    
    // Test: 5d8h = 5d8h1 (default to 1)
    result = dice_roll_expression(ctx, "5d8h");
    TEST_ASSERT(result.success, "Keep h with no value defaults to 1");
    TEST_ASSERT(result.value >= 1 && result.value <= 8, "Default h result in valid range");
    
    // Test: 4d10s5 = 4d10s=5 (default operator to equals)
    result = dice_roll_expression(ctx, "4d10s5");
    TEST_ASSERT(result.success, "Select with no operator defaults to equals");
    
    // Test: 6d6s = 6d6s=1 (default value to 1)
    result = dice_roll_expression(ctx, "6d6s");
    TEST_ASSERT(result.success, "Select with no value defaults to equals 1");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_equivalence_relationships() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    dice_rng_vtable_t rng;
    
    // Test: 'k' and 'h' are equivalent
    rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result1 = dice_roll_expression(ctx, "4d6k3");
    TEST_ASSERT(result1.success, "4d6k3 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(12345);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result2 = dice_roll_expression(ctx, "4d6h3");
    TEST_ASSERT(result2.success, "4d6h3 succeeds");
    TEST_ASSERT(result1.value == result2.value, "'k' and 'h' are equivalent");
    
    // Test: keep high = drop low equivalence (4d6k3 = 4d6l1)
    dice_context_reset(ctx);
    rng = dice_create_system_rng(54321);
    dice_context_set_rng(ctx, &rng);
    
    result1 = dice_roll_expression(ctx, "4d6k3");
    TEST_ASSERT(result1.success, "4d6k3 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(54321);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    result2 = dice_roll_expression(ctx, "4d6l1");
    TEST_ASSERT(result2.success, "4d6l1 succeeds");
    TEST_ASSERT(result1.value == result2.value, "4d6k3 equals 4d6l1");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_edge_cases() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test: Can't drop all dice
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6l3");
    TEST_ASSERT(!result.success, "Cannot drop all dice");
    
    // Reset context after error
    dice_context_reset(ctx);
    
    // Test: Can't drop more than available
    result = dice_roll_expression(ctx, "3d6l5");
    TEST_ASSERT(!result.success, "Cannot drop more dice than rolled");
    
    // Reset context after error
    dice_context_reset(ctx);
    
    // Test: Can't keep more than available
    result = dice_roll_expression(ctx, "3d6k5");
    TEST_ASSERT(!result.success, "Cannot keep more dice than rolled");
    
    // Reset context after error
    dice_context_reset(ctx);
    
    // Test: Drop 0 is valid (same as no drop)
    result = dice_roll_expression(ctx, "3d6l0");
    TEST_ASSERT(result.success, "Drop 0 dice is valid");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Drop 0 result equals sum");
    
    // Test: Keep all is valid
    result = dice_roll_expression(ctx, "3d6k3");
    TEST_ASSERT(result.success, "Keep all dice is valid");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep all result equals sum");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_conditional_selection_improvements() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test: s5 defaults to s=5
    dice_eval_result_t result = dice_roll_expression(ctx, "4d10s5");
    TEST_ASSERT(result.success, "Conditional s5 defaults to s=5");
    
    // Test: s defaults to s=1
    result = dice_roll_expression(ctx, "6d6s");
    TEST_ASSERT(result.success, "Conditional s defaults to s=1");
    
    // Test: incomplete operators still fail
    result = dice_roll_expression(ctx, "3d6s>");
    TEST_ASSERT(!result.success, "Incomplete s> operator should fail");
    
    result = dice_roll_expression(ctx, "3d6s<");
    TEST_ASSERT(!result.success, "Incomplete s< operator should fail");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_case_insensitivity() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test uppercase versions
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6K3");
    TEST_ASSERT(result.success, "Uppercase K works");
    
    result = dice_roll_expression(ctx, "4d6H3");
    TEST_ASSERT(result.success, "Uppercase H works");
    
    result = dice_roll_expression(ctx, "4d6L1");
    TEST_ASSERT(result.success, "Uppercase L works");
    
    result = dice_roll_expression(ctx, "6d6S5");
    TEST_ASSERT(result.success, "Uppercase S works");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_complex_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test selection in arithmetic
    dice_eval_result_t result = dice_roll_expression(ctx, "1d20+4d6k3");
    TEST_ASSERT(result.success, "Selection in arithmetic works");
    TEST_ASSERT(result.value >= 4 && result.value <= 38, "Complex expression in valid range");
    
    // Test multiple different selections
    result = dice_roll_expression(ctx, "4d6k3+3d8l1+6d6s6");
    TEST_ASSERT(result.success, "Multiple different selections work");
    
    // Test with parentheses
    result = dice_roll_expression(ctx, "(4d6k3)*2");
    TEST_ASSERT(result.success, "Selection with parentheses works");
    
    // Test nested expressions
    result = dice_roll_expression(ctx, "2*(3d6k2)+1d4l0");
    TEST_ASSERT(result.success, "Nested expressions with selection work");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Test Runner
// =============================================================================

int main() {
    printf("Running comprehensive new selection syntax tests...\n\n");
    
    RUN_TEST(test_new_syntax_requirements);
    RUN_TEST(test_old_syntax_rejection);
    RUN_TEST(test_default_values);
    RUN_TEST(test_equivalence_relationships);
    RUN_TEST(test_edge_cases);
    RUN_TEST(test_conditional_selection_improvements);
    RUN_TEST(test_case_insensitivity);
    RUN_TEST(test_complex_expressions);
    
    printf("All comprehensive syntax tests passed!\n");
    return 0;
}