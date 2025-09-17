#include "test_common.h"

// =============================================================================
// Dice Selection Tests (Keep/Drop High/Low Operations)
// =============================================================================

int test_basic_selection_operations() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test basic keep high operation (new 'k' syntax)
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6k3");
    TEST_ASSERT(result.success, "Keep high operation succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep high result in valid range");
    
    // Test basic keep low operation (new 'l' syntax)
    result = dice_roll_expression(ctx, "5d6l2");
    TEST_ASSERT(result.success, "Keep low operation succeeds");
    TEST_ASSERT(result.value >= 2 && result.value <= 12, "Keep low result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_case_insensitivity() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test uppercase variants of new syntax
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6K3");
    TEST_ASSERT(result.success, "Uppercase K works");
    
    result = dice_roll_expression(ctx, "4d6H3");
    TEST_ASSERT(result.success, "Uppercase H works");
    
    result = dice_roll_expression(ctx, "4d6L1");
    TEST_ASSERT(result.success, "Uppercase L works");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_equivalence() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test equivalence with fixed seed
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    // Test: 4d6k3 and 4d6l1 are different operations (keep 3 high vs keep 1 low)
    dice_eval_result_t result1 = dice_roll_expression(ctx, "4d6k3");
    TEST_ASSERT(result1.success, "4d6k3 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(12345);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result2 = dice_roll_expression(ctx, "4d6l1");
    TEST_ASSERT(result2.success, "4d6l1 succeeds");
    // Note: These operations are different and will typically produce different results
    
    // Test: 'k' and 'h' are equivalent 
    dice_context_reset(ctx);
    rng = dice_create_system_rng(54321);
    dice_context_set_rng(ctx, &rng);
    
    result1 = dice_roll_expression(ctx, "5d6k2");
    TEST_ASSERT(result1.success, "5d6k2 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(54321);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    result2 = dice_roll_expression(ctx, "5d6h2");
    TEST_ASSERT(result2.success, "5d6h2 succeeds");
    TEST_ASSERT(result1.value == result2.value, "5d6k2 equals 5d6h2 with same seed");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_error_handling() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test keeping more dice than available - now allowed, keeps all dice
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6k5");
    TEST_ASSERT(result.success, "Keep more dice than rolled is now allowed");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep more result equals sum of all dice");
    TEST_ASSERT(!dice_has_error(ctx), "No error flag set for valid keep operation");
    
    dice_context_reset(ctx);
    
    // Test keeping more dice than available - keeps all dice that exist
    result = dice_roll_expression(ctx, "3d6l4");
    TEST_ASSERT(result.success, "Keep more lowest dice than rolled is allowed");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep more lowest result equals sum of all dice");
    TEST_ASSERT(!dice_has_error(ctx), "No error flag set for valid keep operation");
    
    dice_context_reset(ctx);
    
    // Test keeping all dice with 'l' - keeps all 3 dice
    result = dice_roll_expression(ctx, "3d6l3");
    TEST_ASSERT(result.success, "Keep all lowest dice is allowed");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep all lowest result equals sum of all dice");
    TEST_ASSERT(!dice_has_error(ctx), "No error flag set for valid keep operation");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_edge_cases() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test keeping all dice (should work like normal roll)
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6k3");
    TEST_ASSERT(result.success, "Keeping all dice works");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Keep all result in valid range");
    
    // Test keeping zero dice (should return 0)
    result = dice_roll_expression(ctx, "3d6l0");
    TEST_ASSERT(result.success, "Keeping zero dice works");
    TEST_ASSERT(result.value == 0, "Keep zero result equals 0");
    
    // Test single die operations
    result = dice_roll_expression(ctx, "1d20k1");
    TEST_ASSERT(result.success, "Single die keep high works");
    TEST_ASSERT(result.value >= 1 && result.value <= 20, "Single die keep result in valid range");
    
    result = dice_roll_expression(ctx, "1d20l0");
    TEST_ASSERT(result.success, "Single die keep zero works");
    TEST_ASSERT(result.value == 0, "Single die keep zero result equals 0");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_in_complex_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test selection in arithmetic expressions
    dice_eval_result_t result = dice_roll_expression(ctx, "1d20+4d6k3");
    TEST_ASSERT(result.success, "Selection in addition works");
    TEST_ASSERT(result.value >= 4 && result.value <= 38, "Complex expression result in valid range");
    
    // Test multiple selections
    result = dice_roll_expression(ctx, "4d6k3+3d8l1");
    TEST_ASSERT(result.success, "Multiple selections work");
    TEST_ASSERT(result.value >= 5 && result.value <= 34, "Multiple selections result in valid range");
    
    // Test with parentheses
    result = dice_roll_expression(ctx, "(4d6k3)*2");
    TEST_ASSERT(result.success, "Selection with parentheses works");
    TEST_ASSERT(result.value >= 6 && result.value <= 36, "Parenthesized selection result in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_shorthand_syntax() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test basic shorthand operations
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6k3");
    TEST_ASSERT(result.success, "Shorthand 'k' (keep high) operation succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Shorthand keep result in valid range");
    
    result = dice_roll_expression(ctx, "5d6l2");
    TEST_ASSERT(result.success, "Shorthand 'l' (keep low) operation succeeds");
    TEST_ASSERT(result.value >= 2 && result.value <= 12, "Shorthand keep low result in valid range");
    
    // Test uppercase shorthand
    result = dice_roll_expression(ctx, "4d6K3");
    TEST_ASSERT(result.success, "Uppercase shorthand 'K' works");
    
    result = dice_roll_expression(ctx, "5d6L2");
    TEST_ASSERT(result.success, "Uppercase shorthand 'L' works");
    
    // Test 'h' as alias for 'k'
    result = dice_roll_expression(ctx, "4d6h3");
    TEST_ASSERT(result.success, "Shorthand 'h' (keep high alias) operation succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "Shorthand h result in valid range");
    
    result = dice_roll_expression(ctx, "4d6H3");
    TEST_ASSERT(result.success, "Uppercase shorthand 'H' works");
    
    // Test shorthand equivalence with full syntax using fixed seeds
    dice_rng_vtable_t rng = dice_create_system_rng(98765);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result1 = dice_roll_expression(ctx, "3d6k3");  // shorthand for kh3
    TEST_ASSERT(result1.success, "3d6k3 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(98765);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result2 = dice_roll_expression(ctx, "3d6h3");  // h syntax (alias for k)
    TEST_ASSERT(result2.success, "3d6h3 succeeds");
    TEST_ASSERT(result1.value == result2.value, "3d6k3 equals 3d6h3 with same seed");
    
    // Test 'l' (keep low) syntax with fixed seeds
    dice_context_reset(ctx);
    rng = dice_create_system_rng(11111);
    dice_context_set_rng(ctx, &rng);
    
    result1 = dice_roll_expression(ctx, "8d4l4");  // keep 4 lowest
    TEST_ASSERT(result1.success, "8d4l4 succeeds");
    
    dice_context_reset(ctx);
    rng = dice_create_system_rng(11111);  // Same seed
    dice_context_set_rng(ctx, &rng);
    
    result2 = dice_roll_expression(ctx, "8d4k4");  // keep 4 highest (different from l4)
    TEST_ASSERT(result2.success, "8d4k4 succeeds");
    // Note: These operations are different and will typically produce different results
    
    // Test shorthand in complex expressions
    result = dice_roll_expression(ctx, "1d20+4d6k3");
    TEST_ASSERT(result.success, "Shorthand in addition works");
    TEST_ASSERT(result.value >= 4 && result.value <= 38, "Complex expression with shorthand in valid range");
    
    result = dice_roll_expression(ctx, "4d6k3+3d8l1");
    TEST_ASSERT(result.success, "Multiple shorthands work");
    TEST_ASSERT(result.value >= 5 && result.value <= 34, "Multiple shorthands result in valid range");
    
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
    RUN_TEST(test_shorthand_syntax);
    
    printf("All dice selection tests passed!\\n");
    return 0;
}