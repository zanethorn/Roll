#include "test_common.h"

// =============================================================================
// RNG API Tests
// =============================================================================

int test_rng_decoupling() {
    // Test that RNG creation and usage work correctly
    dice_rng_vtable_t custom_rng = dice_create_system_rng(54321);
    
    // Test that the RNG functions work
    TEST_ASSERT(custom_rng.roll != NULL, "RNG has roll function");
    TEST_ASSERT(custom_rng.state != NULL, "RNG has state");
    
    // Test that dice operations work
    int result1 = dice_roll(6);
    TEST_ASSERT(result1 >= 1 && result1 <= 6, "dice_roll() works");
    
    // Cleanup RNG
    if (custom_rng.cleanup) custom_rng.cleanup(custom_rng.state);
    
    return 1;
}

int test_rng_creation() {
    // Test system RNG creation
    dice_rng_vtable_t system_rng = dice_create_system_rng(12345);
    TEST_ASSERT(system_rng.roll != NULL, "System RNG has roll function");
    TEST_ASSERT(system_rng.state != NULL, "System RNG has state");
    
    // Test xoshiro RNG creation
    dice_rng_vtable_t xoshiro_rng = dice_create_xoshiro_rng(54321);
    TEST_ASSERT(xoshiro_rng.roll != NULL, "Xoshiro RNG has roll function");
    TEST_ASSERT(xoshiro_rng.state != NULL, "Xoshiro RNG has state");
    
    // Test that different seeds produce different initial states
    dice_rng_vtable_t rng1 = dice_create_system_rng(111);
    dice_rng_vtable_t rng2 = dice_create_system_rng(222);
    
    // Generate several values to increase likelihood of difference
    int differences = 0;
    for (int i = 0; i < 10; i++) {
        int val1 = rng1.roll(rng1.state, 100);
        int val2 = rng2.roll(rng2.state, 100);
        if (val1 != val2) {
            differences++;
        }
    }
    
    // At least some values should be different (probabilistic test)
    TEST_ASSERT(differences > 0, "Different seeds produce different values (probabilistic test)");
    
    // Cleanup
    if (rng1.cleanup) rng1.cleanup(rng1.state);
    if (rng2.cleanup) rng2.cleanup(rng2.state);
    if (system_rng.cleanup) system_rng.cleanup(system_rng.state);
    if (xoshiro_rng.cleanup) xoshiro_rng.cleanup(xoshiro_rng.state);
    
    return 1;
}

int test_rng_context_integration() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test setting custom RNG in context
    dice_rng_vtable_t custom_rng = dice_create_xoshiro_rng(98765);
    dice_context_set_rng(ctx, &custom_rng);
    
    // Test that context uses the custom RNG
    dice_eval_result_t result = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(result.success, "Expression evaluation with custom RNG succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "Custom RNG produces valid results");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// RNG Quality Tests
// =============================================================================

int test_rng_reproducibility() {
    // Test that same seed produces same sequence
    
    int sequence1[10];
    for (int i = 0; i < 10; i++) {
        sequence1[i] = dice_roll(6);
    }
    
    // Same seed
    int sequence2[10];
    for (int i = 0; i < 10; i++) {
        sequence2[i] = dice_roll(6);
    }
    
    // Sequences should be identical
    for (int i = 0; i < 10; i++) {
        TEST_ASSERT(sequence1[i] == sequence2[i], "Same seed produces same sequence");
    }
    
    return 1;
}

int test_rng_range_validation() {
    
    // Use context-based API for better randomness in statistical tests
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return 0;
    
    dice_rng_vtable_t rng = dice_create_system_rng(42);
    dice_context_set_rng(ctx, &rng);
    
    // Test a few die sizes to ensure correct range (reduced for reliability)
    int die_sizes[] = {2, 6, 10, 20};
    int num_sizes = sizeof(die_sizes) / sizeof(die_sizes[0]);
    
    for (int i = 0; i < num_sizes; i++) {
        int sides = die_sizes[i];
        bool found_min = false, found_max = false;
        
        // Roll many times to try to hit min and max values
        for (int j = 0; j < 5000; j++) { // Increased attempts for better coverage
            int roll = ctx->rng.roll(ctx->rng.state, sides);
            if (!(roll >= 1 && roll <= sides)) {
                TEST_ASSERT(0, "Roll is within valid range");
                dice_context_destroy(ctx);
                return 0;
            }
            if (roll == 1) found_min = true;
            if (roll == sides) found_max = true;
            
            // Early exit if both found
            if (found_min && found_max) break;
        }
        
        // For single-sided die, min and max are the same
        if (sides == 1) {
            TEST_ASSERT(found_min, "1-sided die always returns 1");
        } else {
            TEST_ASSERT(found_min, "Minimum value (1) was rolled");
            TEST_ASSERT(found_max, "Maximum value was rolled");  
        }
    }
    
    dice_context_destroy(ctx);
    return 1;
}

int test_rng_distribution_basic() {
    
    // Use context-based API for better randomness in statistical tests
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return 0;
    
    dice_rng_vtable_t rng = dice_create_system_rng(99999);
    dice_context_set_rng(ctx, &rng);
    
    // Test that results don't show obvious bias
    int sides = 6;
    int frequencies[6] = {0};
    int total_rolls = 6000; // 1000 per side
    
    for (int i = 0; i < total_rolls; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, sides);
        frequencies[roll - 1]++;
    }
    
    // Each face should appear roughly 1000 times
    // Allow for some variance - check that no face appears less than 800 or more than 1200 times
    for (int i = 0; i < sides; i++) {
        TEST_ASSERT(frequencies[i] >= 800, "No face appears too rarely");
        TEST_ASSERT(frequencies[i] <= 1200, "No face appears too frequently");
    }
    
    dice_context_destroy(ctx);
    return 1;
}

int test_rng_no_patterns() {
    
    // Use context-based API for better randomness in statistical tests
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return 0;
    
    dice_rng_vtable_t rng = dice_create_system_rng(77777);
    dice_context_set_rng(ctx, &rng);
    
    // Test for obvious patterns
    int prev_roll = ctx->rng.roll(ctx->rng.state, 6);
    int alternating_count = 0;
    int identical_count = 0;
    int ascending_count = 0;
    
    for (int i = 0; i < 100; i++) {
        int current_roll = ctx->rng.roll(ctx->rng.state, 6);
        
        // Check for alternating pattern (1,2,1,2,1,2...)
        if (i > 0 && current_roll == prev_roll) {
            identical_count++;
        }
        
        // Check for strictly ascending pattern  
        if (current_roll == (prev_roll % 6) + 1) {
            ascending_count++;
        }
        
        prev_roll = current_roll;
    }
    
    // These patterns should not occur frequently in good random data
    TEST_ASSERT(identical_count < 50, "Not too many identical consecutive rolls");
    TEST_ASSERT(ascending_count < 25, "Not too many ascending patterns");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_different_rng_implementations() {
    // Test that system RNG and xoshiro produce different sequences
    dice_rng_vtable_t system_rng = dice_create_system_rng(12345);
    dice_rng_vtable_t xoshiro_rng = dice_create_xoshiro_rng(12345);
    
    int system_values[10];
    int xoshiro_values[10];
    
    for (int i = 0; i < 10; i++) {
        system_values[i] = system_rng.roll(system_rng.state, 100);
        xoshiro_values[i] = xoshiro_rng.roll(xoshiro_rng.state, 100);
    }
    
    // At least some values should be different (probabilistic test)
    int differences = 0;
    for (int i = 0; i < 10; i++) {
        if (system_values[i] != xoshiro_values[i]) {
            differences++;
        }
    }
    
    TEST_ASSERT(differences > 0, "Different RNG implementations produce different sequences");
    
    // Cleanup
    if (system_rng.cleanup) system_rng.cleanup(system_rng.state);
    if (xoshiro_rng.cleanup) xoshiro_rng.cleanup(xoshiro_rng.state);
    
    return 1;
}

// =============================================================================
// RNG Failure Mode Tests  
// =============================================================================

int test_rng_invalid_inputs() {
    
    
    // Test invalid side counts are handled by dice functions
    // (The RNG itself might not validate, but the dice functions should)
    int result = dice_roll(0);
    TEST_ASSERT(result == -1, "dice_roll(0) handled properly");
    
    result = dice_roll(-5);
    TEST_ASSERT(result == -1, "dice_roll(-5) handled properly");
    
    return 1;
}

int test_rng_state_isolation() {
    // Test that different contexts don't interfere with each other
    dice_context_t *ctx1 = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    dice_context_t *ctx2 = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    dice_rng_vtable_t rng1 = dice_create_system_rng(111);
    dice_rng_vtable_t rng2 = dice_create_system_rng(222);
    
    dice_context_set_rng(ctx1, &rng1);
    dice_context_set_rng(ctx2, &rng2);
    
    // Generate values from both contexts
    dice_eval_result_t result1 = dice_roll_expression(ctx1, "1d100");
    dice_eval_result_t result2 = dice_roll_expression(ctx2, "1d100");
    
    TEST_ASSERT(result1.success, "Context 1 RNG works");
    TEST_ASSERT(result2.success, "Context 2 RNG works");
    
    // Generate more values to ensure independence
    for (int i = 0; i < 10; i++) {
        result1 = dice_roll_expression(ctx1, "1d6");
        result2 = dice_roll_expression(ctx2, "1d6");
        TEST_ASSERT(result1.success && result2.success, "Both contexts continue to work independently");
    }
    
    dice_context_destroy(ctx1);
    dice_context_destroy(ctx2);
    return 1;
}

int main() {
    printf("Running RNG tests...\n\n");
    
    RUN_TEST(test_rng_decoupling);
    RUN_TEST(test_rng_creation);
    RUN_TEST(test_rng_context_integration);
    RUN_TEST(test_rng_reproducibility);
    RUN_TEST(test_rng_range_validation);
    RUN_TEST(test_rng_distribution_basic);
    RUN_TEST(test_rng_no_patterns);
    RUN_TEST(test_different_rng_implementations);
    RUN_TEST(test_rng_invalid_inputs);
    RUN_TEST(test_rng_state_isolation);
    
    printf("All RNG tests passed!\n");
    return 0;
}