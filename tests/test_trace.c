#include "test_common.h"

// =============================================================================
// Trace Functionality Tests
// =============================================================================

int test_basic_trace_functionality() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test that trace is initially empty
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace != NULL, "Trace is available from new context");
    TEST_ASSERT(trace->count == 0, "New context has empty trace");
    
    // Evaluate expression and check trace
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6");
    TEST_ASSERT(result.success, "2d6 evaluation succeeds");
    
    trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count >= 2, "Trace contains at least 2 atomic rolls");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_content_structure() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test simple die roll trace
    dice_eval_result_t result = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(result.success, "1d6 evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count >= 1, "Single die roll produces trace entry");
    
    // Check trace structure
    TEST_ASSERT(trace->first != NULL, "Trace has first entry");
    TEST_ASSERT(trace->last != NULL, "Trace has last entry");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_multiple_dice() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test multiple dice trace
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6");
    TEST_ASSERT(result.success, "3d6 evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count >= 3, "3d6 produces at least 3 trace entries");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_complex_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test complex expression trace  
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6+1d4+3");
    TEST_ASSERT(result.success, "Complex expression evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count >= 3, "Complex expression produces multiple trace entries");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_clearing() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Generate some trace data
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6");
    TEST_ASSERT(result.success, "Expression evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count > 0, "Trace contains entries before clearing");
    
    // Clear trace
    dice_clear_trace(ctx);
    
    trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count == 0, "Trace is empty after clearing");
    TEST_ASSERT(trace->first == NULL, "Trace first pointer is NULL after clearing");
    TEST_ASSERT(trace->last == NULL, "Trace last pointer is NULL after clearing");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_persistence_across_evaluations() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // First evaluation
    dice_eval_result_t result1 = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(result1.success, "First evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    size_t count_after_first = trace->count;
    TEST_ASSERT(count_after_first > 0, "First evaluation produces trace entries");
    
    // Second evaluation (without clearing)
    dice_eval_result_t result2 = dice_roll_expression(ctx, "1d8");
    TEST_ASSERT(result2.success, "Second evaluation succeeds");
    
    trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count > count_after_first, "Trace accumulates across evaluations");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_with_arithmetic_only() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test pure arithmetic expression
    dice_eval_result_t result = dice_roll_expression(ctx, "2+3*4");
    TEST_ASSERT(result.success, "Arithmetic expression evaluation succeeds");
    TEST_ASSERT(result.value == 14, "Arithmetic result is correct");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    // Arithmetic-only expressions may or may not generate trace entries
    // depending on implementation - either behavior is acceptable
    TEST_ASSERT(trace != NULL, "Trace is accessible after arithmetic expression");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_error_conditions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test trace with invalid expression
    dice_eval_result_t result = dice_roll_expression(ctx, "invalid");
    TEST_ASSERT(!result.success, "Invalid expression fails");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace != NULL, "Trace is accessible even after failed evaluation");
    // Trace count could be 0 (no entries for failed parse) or > 0 (partial entries)
    // Both are acceptable depending on when the failure occurs
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Trace Edge Cases
// =============================================================================

int test_trace_with_null_context() {
    // Test trace functions with NULL context
    const dice_trace_t *trace = dice_get_trace(NULL);
    TEST_ASSERT(trace == NULL, "dice_get_trace(NULL) returns NULL");
    
    // dice_clear_trace with NULL should not crash
    dice_clear_trace(NULL);
    TEST_ASSERT(1, "dice_clear_trace(NULL) doesn't crash");
    
    return 1;
}

int test_trace_memory_usage() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Generate many trace entries to test memory usage
    for (int i = 0; i < 100; i++) {
        dice_eval_result_t result = dice_roll_expression(ctx, "1d6");
        TEST_ASSERT(result.success, "Repeated evaluations succeed");
        
        // Periodically check trace
        const dice_trace_t *trace = dice_get_trace(ctx);
        TEST_ASSERT(trace->count > 0, "Trace continues to accumulate");
        
        // Clear trace occasionally to prevent memory exhaustion
        if (i % 20 == 0) {
            dice_clear_trace(ctx);
        }
    }
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_consistency() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test that trace remains consistent across multiple accesses
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6");
    TEST_ASSERT(result.success, "Expression evaluation succeeds");
    
    const dice_trace_t *trace1 = dice_get_trace(ctx);
    const dice_trace_t *trace2 = dice_get_trace(ctx);
    
    TEST_ASSERT(trace1 == trace2, "Multiple calls to dice_get_trace return same pointer");
    TEST_ASSERT(trace1->count == trace2->count, "Trace count is consistent");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_after_clear_and_reuse() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Generate trace, clear it, then generate more
    dice_eval_result_t result1 = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(result1.success, "First evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count > 0, "Initial trace has entries");
    
    dice_clear_trace(ctx);
    
    dice_eval_result_t result2 = dice_roll_expression(ctx, "1d8");
    TEST_ASSERT(result2.success, "Second evaluation after clear succeeds");
    
    trace = dice_get_trace(ctx);
    TEST_ASSERT(trace->count > 0, "New trace entries created after clear");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Trace Policy Integration Tests
// =============================================================================

int test_trace_with_policy_violations() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set restrictive policy
    dice_policy_t policy = dice_default_policy();
    policy.max_dice_count = 2;
    dice_context_set_policy(ctx, &policy);
    
    // Try expression that violates policy
    dice_eval_result_t result = dice_roll_expression(ctx, "5d6");  // Should fail
    TEST_ASSERT(!result.success, "Policy violation causes failure");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set for policy violation");
    
    // Check that trace is still accessible
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace != NULL, "Trace is accessible even after policy violation");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_with_different_features() {
    // Test trace behavior with different feature sets
    dice_context_t *basic_ctx = dice_context_create(64 * 1024, DICE_FEATURE_BASIC);
    dice_context_t *all_ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test basic dice with both contexts
    dice_eval_result_t basic_result = dice_roll_expression(basic_ctx, "1d6");
    dice_eval_result_t all_result = dice_roll_expression(all_ctx, "1d6");
    
    TEST_ASSERT(basic_result.success, "Basic context handles simple dice");
    TEST_ASSERT(all_result.success, "All-features context handles simple dice");
    
    // Both should have trace functionality
    const dice_trace_t *basic_trace = dice_get_trace(basic_ctx);
    const dice_trace_t *all_trace = dice_get_trace(all_ctx);
    
    TEST_ASSERT(basic_trace != NULL, "Basic context provides trace");
    TEST_ASSERT(all_trace != NULL, "All-features context provides trace");
    
    dice_context_destroy(basic_ctx);
    dice_context_destroy(all_ctx);
    return 1;
}

int main() {
    printf("Running trace functionality tests...\n\n");
    
    RUN_TEST(test_basic_trace_functionality);
    RUN_TEST(test_trace_content_structure);
    RUN_TEST(test_trace_multiple_dice);
    RUN_TEST(test_trace_complex_expressions);
    RUN_TEST(test_trace_clearing);
    RUN_TEST(test_trace_persistence_across_evaluations);
    RUN_TEST(test_trace_with_arithmetic_only);
    RUN_TEST(test_trace_error_conditions);
    RUN_TEST(test_trace_with_null_context);
    RUN_TEST(test_trace_memory_usage);
    RUN_TEST(test_trace_consistency);
    RUN_TEST(test_trace_after_clear_and_reuse);
    RUN_TEST(test_trace_with_policy_violations);
    RUN_TEST(test_trace_with_different_features);
    
    printf("All trace tests passed!\n");
    return 0;
}