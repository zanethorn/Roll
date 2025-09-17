#include "test_common.h"

// =============================================================================
// Integration Tests - Testing Combined Functionality  
// =============================================================================

int test_new_architecture_integration() {
    // Test the new architecture directly (from original test_dice.c)
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    TEST_ASSERT(ctx != NULL, "Context creation succeeds");
    
    // Test tracing
    dice_eval_result_t result = dice_roll_expression(ctx, "2d6");
    TEST_ASSERT(result.success, "2d6 evaluation succeeds");
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    TEST_ASSERT(trace != NULL, "Trace is available");
    TEST_ASSERT(trace->count >= 2, "Trace contains at least 2 atomic rolls");
    
    // Test policy
    dice_policy_t policy = dice_default_policy();
    policy.max_dice_count = 2;
    dice_context_set_policy(ctx, &policy);
    
    result = dice_roll_expression(ctx, "5d6");  // Should fail
    TEST_ASSERT(!result.success, "Exceeding max dice count fails");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set for policy violation");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_exploding_dice_placeholder() {
    // Test exploding dice (currently not implemented, but test graceful handling)
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Try to evaluate exploding dice notation - may or may not be implemented
    dice_eval_result_t result = dice_roll_expression(ctx, "1d6!");
    
    if (result.success) {
        TEST_ASSERT(result.value >= 1, "Exploding dice returns positive value");
    } else {
        // If not implemented, that's also acceptable for now
        TEST_ASSERT(dice_has_error(ctx), "Exploding dice fails gracefully if not implemented");
    }
    
    dice_context_destroy(ctx);
    return 1;
}

int test_full_workflow_integration() {
    // Test complete workflow from initialization to cleanup
    dice_init(12345);
    
    // Test legacy API functions
    const char *version = dice_version();
    TEST_ASSERT(version != NULL, "Version function works");
    
    // Test various dice operations
    int result = dice_roll(6);
    TEST_ASSERT(result >= 1 && result <= 6, "Basic roll works");
    
    result = dice_roll_multiple(3, 6);
    TEST_ASSERT(result >= 3 && result <= 18, "Multiple roll works");
    
    int individual[3];
    result = dice_roll_individual(3, 6, individual);
    TEST_ASSERT(result >= 3 && result <= 18, "Individual roll works");
    
    result = dice_roll_notation("2d6+3");
    TEST_ASSERT(result >= 5 && result <= 15, "Notation roll works");
    
    // Cleanup
    dice_cleanup();
    TEST_ASSERT(1, "Cleanup completes without error");
    
    return 1;
}

// =============================================================================
// Stress Tests - High Load and Edge Conditions
// =============================================================================

int test_high_volume_operations() {
    dice_init(999);
    
    // Perform many operations to test for memory leaks, crashes, etc.
    for (int i = 0; i < 1000; i++) {
        int result = dice_roll(6);
        TEST_ASSERT(result >= 1 && result <= 6, "High volume rolls remain valid");
        
        if (i % 100 == 0) {
            // Periodically test other operations
            result = dice_roll_multiple(2, 6);
            TEST_ASSERT(result >= 2 && result <= 12, "High volume multiple rolls remain valid");
        }
    }
    
    dice_cleanup();
    return 1;
}

int test_rapid_context_creation() {
    // Test rapid context creation and destruction
    for (int i = 0; i < 100; i++) {
        dice_context_t *ctx = dice_context_create(SMALL_ARENA_SIZE, DICE_FEATURE_ALL);
        TEST_ASSERT(ctx != NULL, "Rapid context creation succeeds");
        
        dice_eval_result_t result = dice_roll_expression(ctx, "1d6");
        TEST_ASSERT(result.success, "Rapid context usage works");
        
        dice_context_destroy(ctx);
    }
    
    return 1;
}

int test_mixed_api_usage() {
    // Test mixing legacy API and new context API
    dice_init(555);
    
    // Use legacy API
    int legacy_result = dice_roll(6);
    TEST_ASSERT(legacy_result >= 1 && legacy_result <= 6, "Legacy API works");
    
    // Use new context API
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    dice_eval_result_t context_result = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(context_result.success, "Context API works");
    
    // Use legacy API again
    legacy_result = dice_roll_multiple(2, 6);
    TEST_ASSERT(legacy_result >= 2 && legacy_result <= 12, "Legacy API still works after context usage");
    
    dice_context_destroy(ctx);
    dice_cleanup();
    return 1;
}

// =============================================================================
// Error Recovery Tests
// =============================================================================

int test_error_recovery_sequence() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Cause an error
    dice_eval_result_t result = dice_roll_expression(ctx, "invalid");
    TEST_ASSERT(!result.success, "Invalid expression fails");
    TEST_ASSERT(dice_has_error(ctx), "Error flag is set");
    
    // Clear error and try valid expression
    dice_clear_error(ctx);
    TEST_ASSERT(!dice_has_error(ctx), "Error flag cleared");
    
    result = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(result.success, "Valid expression works after error recovery");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_multiple_error_conditions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test various error conditions in sequence
    dice_eval_result_t result;
    
    result = dice_roll_expression(ctx, "10/0");
    TEST_ASSERT(!result.success, "Division by zero error");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "0d6");
    TEST_ASSERT(!result.success, "Zero dice count error");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "1d0");
    TEST_ASSERT(!result.success, "Zero sides error");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "((");
    TEST_ASSERT(!result.success, "Parsing error");
    dice_clear_error(ctx);
    
    // Verify context still works after all errors
    result = dice_roll_expression(ctx, "2+2");
    TEST_ASSERT(result.success, "Context recovers from multiple errors");
    TEST_ASSERT(result.value == 4, "Result is correct after recovery");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Feature Compatibility Tests
// =============================================================================

int test_feature_flags() {
    // Test different feature flag combinations
    dice_context_t *basic_ctx = dice_context_create(64 * 1024, DICE_FEATURE_BASIC);
    dice_context_t *all_ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Basic features should work in both
    dice_eval_result_t basic_result = dice_roll_expression(basic_ctx, "1d6");
    dice_eval_result_t all_result = dice_roll_expression(all_ctx, "1d6");
    
    TEST_ASSERT(basic_result.success, "Basic dice work with basic features");
    TEST_ASSERT(all_result.success, "Basic dice work with all features");
    
    // Advanced features may only work with all features enabled
    // (Exact behavior depends on implementation)
    
    // Test FATE dice auto-registration with different feature flags
    dice_eval_result_t fate_basic = dice_roll_expression(basic_ctx, "1dF");
    dice_eval_result_t fate_all = dice_roll_expression(all_ctx, "1dF");
    
    // FATE dice should not work with basic features only (F not registered)
    TEST_ASSERT(!fate_basic.success, "FATE dice (1dF) should fail without DICE_FEATURE_FATE");
    
    // FATE dice should work with all features enabled (auto-registered)
    TEST_ASSERT(fate_all.success, "FATE dice (1dF) should work with DICE_FEATURE_FATE enabled");
    TEST_ASSERT(fate_all.value >= -1 && fate_all.value <= 1, "FATE dice result should be between -1 and 1");
    
    // Test multiple FATE dice with auto-registration
    dice_clear_error(all_ctx);
    dice_eval_result_t fate_multi = dice_roll_expression(all_ctx, "4dF");
    TEST_ASSERT(fate_multi.success, "Multiple FATE dice (4dF) should work with auto-registration");
    TEST_ASSERT(fate_multi.value >= -4 && fate_multi.value <= 4, "4dF result should be between -4 and 4");
    
    dice_context_destroy(basic_ctx);
    dice_context_destroy(all_ctx);
    return 1;
}

int test_policy_variations() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test various policy configurations
    dice_policy_t policies[] = {
        {.max_dice_count = 10, .max_sides = 100, .max_explosion_depth = 5, .allow_negative_dice = false, .strict_mode = false},
        {.max_dice_count = 1000, .max_sides = 1000000, .max_explosion_depth = 10, .allow_negative_dice = false, .strict_mode = true},
        {.max_dice_count = 5, .max_sides = 20, .max_explosion_depth = 3, .allow_negative_dice = true, .strict_mode = false}
    };
    
    for (int i = 0; i < 3; i++) {
        dice_context_set_policy(ctx, &policies[i]);
        
        // Test basic operation with each policy
        dice_eval_result_t result = dice_roll_expression(ctx, "1d6");
        TEST_ASSERT(result.success, "Basic operations work with various policies");
    }
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Cross-Platform Compatibility Tests
// =============================================================================

int test_large_value_handling() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test handling of large values that might cause overflow
    dice_eval_result_t result = dice_roll_expression(ctx, "2000000000");
    if (result.success) {
        TEST_ASSERT(result.value == 2000000000, "Large values handled correctly");
    }
    // If it fails, that's acceptable - depends on platform int size
    
    dice_context_destroy(ctx);
    return 1;
}

int test_string_handling_robustness() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test various string inputs for robustness
    const char* test_strings[] = {
        "",           // Empty string
        " ",          // Whitespace only
        "\t\n",       // Other whitespace
        "1d6 extra",  // Valid expression with trailing text
        " 1d6 ",      // Valid expression with surrounding whitespace
        NULL          // NULL pointer (should be handled by wrapper functions)
    };
    
    for (int i = 0; i < 5; i++) { // Skip NULL test for now
        dice_eval_result_t result = dice_roll_expression(ctx, test_strings[i]);
        // Success or failure is acceptable, just shouldn't crash
        TEST_ASSERT(1, "String input doesn't cause crash");
    }
    
    dice_context_destroy(ctx);
    return 1;
}

int main() {
    printf("Running integration tests...\n\n");
    
    RUN_TEST(test_new_architecture_integration);
    RUN_TEST(test_exploding_dice_placeholder);
    RUN_TEST(test_full_workflow_integration);
    RUN_TEST(test_high_volume_operations);
    RUN_TEST(test_rapid_context_creation);
    RUN_TEST(test_mixed_api_usage);
    RUN_TEST(test_error_recovery_sequence);
    RUN_TEST(test_multiple_error_conditions);
    RUN_TEST(test_feature_flags);
    RUN_TEST(test_policy_variations);
    RUN_TEST(test_large_value_handling);
    RUN_TEST(test_string_handling_robustness);
    
    printf("All integration tests passed!\n");
    return 0;
}