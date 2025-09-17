#include "test_common.h"

// =============================================================================
// Reroll Tests (r, r1, r>N, r<N, r>=N, r<=N, r=N, r<>N)
// =============================================================================

int test_basic_reroll() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test basic reroll (should reroll 1s)
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6r");
    TEST_ASSERT(result.success, "Basic reroll r succeeds");
    TEST_ASSERT(result.value >= 6 && result.value <= 18, "r result in valid range (no 1s)");
    
    // Test explicit reroll 1s
    result = dice_roll_expression(ctx, "3d6r1");
    TEST_ASSERT(result.success, "Reroll r1 succeeds");
    TEST_ASSERT(result.value >= 6 && result.value <= 18, "r1 result in valid range (no 1s)");
    
    // Test reroll specific number
    result = dice_roll_expression(ctx, "3d6r6");
    TEST_ASSERT(result.success, "Reroll r6 succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 15, "r6 result in valid range (no 6s)");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_conditional_operators() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test greater than
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6r>4");
    TEST_ASSERT(result.success, "Reroll r>4 succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 12, "r>4 result in valid range (only 1-4)");
    
    // Test less than
    result = dice_roll_expression(ctx, "3d6r<3");
    TEST_ASSERT(result.success, "Reroll r<3 succeeds");
    TEST_ASSERT(result.value >= 9 && result.value <= 18, "r<3 result in valid range (only 3-6)");
    
    // Test greater than or equal
    result = dice_roll_expression(ctx, "3d6r>=5");
    TEST_ASSERT(result.success, "Reroll r>=5 succeeds");
    TEST_ASSERT(result.value >= 3 && result.value <= 12, "r>=5 result in valid range (only 1-4)");
    
    // Test less than or equal
    result = dice_roll_expression(ctx, "3d6r<=2");
    TEST_ASSERT(result.success, "Reroll r<=2 succeeds");
    TEST_ASSERT(result.value >= 9 && result.value <= 18, "r<=2 result in valid range (only 3-6)");
    
    // Test inequality (reroll anything not equal to 3)
    result = dice_roll_expression(ctx, "3d6r<>3");
    TEST_ASSERT(result.success, "Reroll r<>3 succeeds");
    TEST_ASSERT(result.value == 9, "r<>3 result should be exactly 9 (only 3s)");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_with_different_dice() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test with d20
    dice_eval_result_t result = dice_roll_expression(ctx, "1d20r1");
    TEST_ASSERT(result.success, "d20 reroll succeeds");
    TEST_ASSERT(result.value >= 2 && result.value <= 20, "d20r1 result in valid range (no 1s)");
    
    // Test with d4
    result = dice_roll_expression(ctx, "2d4r4");
    TEST_ASSERT(result.success, "d4 reroll succeeds");
    TEST_ASSERT(result.value >= 2 && result.value <= 6, "d4r4 result in valid range (no 4s)");
    
    // Test with d8
    result = dice_roll_expression(ctx, "1d8r>=7");
    TEST_ASSERT(result.success, "d8 reroll succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "d8r>=7 result in valid range (only 1-6)");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_edge_cases() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test reroll with impossible condition (would cause infinite loop)
    // r>=1 would reroll everything - should hit safety limit
    dice_eval_result_t result = dice_roll_expression(ctx, "1d6r>=1");
    TEST_ASSERT(!result.success, "Impossible reroll condition should fail");
    TEST_ASSERT(dice_has_error(ctx), "Error should be set for impossible reroll");
    dice_clear_error(ctx);
    
    // Test reroll condition that never triggers
    result = dice_roll_expression(ctx, "3d6r>6");
    TEST_ASSERT(result.success, "Reroll r>6 succeeds (no rerolls needed)");
    TEST_ASSERT(result.value >= 3 && result.value <= 18, "r>6 result in valid range");
    
    // Test reroll with single die
    result = dice_roll_expression(ctx, "1d6r1");
    TEST_ASSERT(result.success, "Single die reroll succeeds");
    TEST_ASSERT(result.value >= 2 && result.value <= 6, "Single die r1 in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_with_arithmetic() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test reroll in arithmetic expression
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6r1+5");
    TEST_ASSERT(result.success, "Reroll with addition succeeds");
    TEST_ASSERT(result.value >= 9 && result.value <= 17, "r1+5 result in valid range");
    
    // Test multiple reroll expressions
    result = dice_roll_expression(ctx, "1d6r1+1d6r6");
    TEST_ASSERT(result.success, "Multiple reroll expressions succeed");
    TEST_ASSERT(result.value >= 3 && result.value <= 11, "Multiple rerolls in valid range");
    
    // Test reroll with multiplication
    result = dice_roll_expression(ctx, "2*(1d6r1)");
    TEST_ASSERT(result.success, "Reroll with multiplication succeeds");
    TEST_ASSERT(result.value >= 4 && result.value <= 12, "Multiplication with reroll in valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_deterministic() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set deterministic seed for predictable results
    dice_rng_vtable_t rng = dice_create_system_rng(42);
    dice_context_set_rng(ctx, &rng);
    
    // Test with known seed
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6r1");
    TEST_ASSERT(result.success, "Deterministic reroll succeeds");
    
    // Reset to same seed and verify same result
    dice_context_destroy(ctx);
    ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    rng = dice_create_system_rng(42);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result2 = dice_roll_expression(ctx, "3d6r1");
    TEST_ASSERT(result2.success, "Second deterministic reroll succeeds");
    TEST_ASSERT(result.value == result2.value, "Deterministic reroll produces same result");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_trace() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set deterministic seed for predictable trace
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6r1");
    TEST_ASSERT(result.success, "Reroll with trace succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace != NULL, "Trace is available");
    TEST_ASSERT(trace->count >= 2, "At least 2 dice results in trace");
    
    // Check that some dice are marked as selected (final results)
    bool found_selected = false;
    const dice_trace_entry_t *entry = trace->first;
    while (entry) {
        if (entry->type == TRACE_ATOMIC_ROLL && entry->data.atomic_roll.selected) {
            found_selected = true;
            break;
        }
        entry = entry->next;
    }
    TEST_ASSERT(found_selected, "Some dice marked as selected in trace");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_reroll_syntax_errors() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test incomplete reroll operator
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6r>");
    TEST_ASSERT(!result.success, "Incomplete r> should fail");
    TEST_ASSERT(dice_has_error(ctx), "Error should be set for incomplete r>");
    dice_clear_error(ctx);
    
    // Test invalid reroll operator
    result = dice_roll_expression(ctx, "3d6r<");
    TEST_ASSERT(!result.success, "Incomplete r< should fail");
    TEST_ASSERT(dice_has_error(ctx), "Error should be set for incomplete r<");
    dice_clear_error(ctx);
    
    dice_context_destroy(ctx);
    return 1;
}

int main() {
    printf("Running reroll operator tests...\n\n");
    
    RUN_TEST(test_basic_reroll);
    RUN_TEST(test_reroll_conditional_operators);
    RUN_TEST(test_reroll_with_different_dice);
    RUN_TEST(test_reroll_edge_cases);
    RUN_TEST(test_reroll_with_arithmetic);
    RUN_TEST(test_reroll_deterministic);
    RUN_TEST(test_reroll_trace);
    RUN_TEST(test_reroll_syntax_errors);
    
    printf("All reroll tests passed!\n");
    return 0;
}