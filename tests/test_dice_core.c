#include "test_common.h"

// =============================================================================
// Core Dice Operation Tests
// =============================================================================

int test_dice_version() {
    const char *version = dice_version();
    TEST_ASSERT(version != NULL, "dice_version() returns non-null");
    TEST_ASSERT(strlen(version) > 0, "dice_version() returns non-empty string");
    printf("Library version: %s\n", version);
    return 1;
}



int test_dice_roll() {
    
    // Test valid rolls
    int result = dice_roll(6);
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll(6) returns value between 1 and 6");
    
    result = dice_roll(20);
    TEST_ASSERT(result >= 1 && result <= 20, "dice_roll(20) returns value between 1 and 20");
    
    // Test invalid inputs (negative tests)
    result = dice_roll(0);
    TEST_ASSERT(result == -1, "dice_roll(0) returns -1");
    
    result = dice_roll(-5);
    TEST_ASSERT(result == -1, "dice_roll(-5) returns -1");
    
    // Test very large values
    result = dice_roll(1000000);
    TEST_ASSERT(result >= 1 && result <= 1000000, "dice_roll(1000000) returns valid range");
    
    return 1;
}

int test_dice_roll_multiple() {
    
    // Test valid multiple rolls
    int result = dice_roll_multiple(3, 6);
    TEST_ASSERT(result >= 3 && result <= 18, "dice_roll_multiple(3, 6) returns value between 3 and 18");
    
    // Test single die
    result = dice_roll_multiple(1, 20);
    TEST_ASSERT(result >= 1 && result <= 20, "dice_roll_multiple(1, 20) returns value between 1 and 20");
    
    // Test invalid inputs (negative tests)
    result = dice_roll_multiple(0, 6);
    TEST_ASSERT(result == -1, "dice_roll_multiple(0, 6) returns -1");
    
    result = dice_roll_multiple(3, 0);
    TEST_ASSERT(result == -1, "dice_roll_multiple(3, 0) returns -1");
    
    result = dice_roll_multiple(-1, 6);
    TEST_ASSERT(result == -1, "dice_roll_multiple(-1, 6) returns -1");
    
    result = dice_roll_multiple(3, -6);
    TEST_ASSERT(result == -1, "dice_roll_multiple(3, -6) returns -1");
    
    // Test edge cases
    result = dice_roll_multiple(1, 1);
    TEST_ASSERT(result == 1, "dice_roll_multiple(1, 1) returns 1");
    
    return 1;
}

int test_dice_roll_individual() {
    
    int results[3];
    
    // Test valid individual rolls
    int sum = dice_roll_individual(3, 6, results);
    TEST_ASSERT(sum >= 3 && sum <= 18, "dice_roll_individual(3, 6) returns sum between 3 and 18");
    
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(results[i] >= 1 && results[i] <= 6, "Individual die result is between 1 and 6");
    }
    
    // Check sum matches individual results
    int manual_sum = results[0] + results[1] + results[2];
    TEST_ASSERT(sum == manual_sum, "Sum matches individual results");
    
    // Test invalid inputs (negative tests)
    int fail_result = dice_roll_individual(3, 6, NULL);
    TEST_ASSERT(fail_result == -1, "dice_roll_individual() with NULL results returns -1");
    
    fail_result = dice_roll_individual(0, 6, results);
    TEST_ASSERT(fail_result == -1, "dice_roll_individual(0, 6) returns -1");
    
    fail_result = dice_roll_individual(3, 0, results);
    TEST_ASSERT(fail_result == -1, "dice_roll_individual(3, 0) returns -1");
    
    fail_result = dice_roll_individual(-1, 6, results);
    TEST_ASSERT(fail_result == -1, "dice_roll_individual(-1, 6) returns -1");
    
    return 1;
}

int test_dice_roll_notation() {
    
    // Test basic notation
    int result = dice_roll_notation("1d6");
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll_notation('1d6') returns value between 1 and 6");
    
    result = dice_roll_notation("3d6");
    TEST_ASSERT(result >= 3 && result <= 18, "dice_roll_notation('3d6') returns value between 3 and 18");
    
    // Test with modifiers
    result = dice_roll_notation("1d6+5");
    TEST_ASSERT(result >= 6 && result <= 11, "dice_roll_notation('1d6+5') returns value between 6 and 11");
    
    result = dice_roll_notation("1d6-1");
    TEST_ASSERT(result >= 0 && result <= 5, "dice_roll_notation('1d6-1') returns value between 0 and 5");
    
    // Test uppercase D
    result = dice_roll_notation("1D6");
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll_notation('1D6') returns value between 1 and 6");
    
    // Test arithmetic expressions
    result = dice_roll_notation("2*3");
    TEST_ASSERT(result == 6, "dice_roll_notation('2*3') returns 6");
    
    result = dice_roll_notation("10/2");
    TEST_ASSERT(result == 5, "dice_roll_notation('10/2') returns 5");
    
    result = dice_roll_notation("(2+3)*4");
    TEST_ASSERT(result == 20, "dice_roll_notation('(2+3)*4') returns 20");
    
    result = dice_roll_notation("-5+10");
    TEST_ASSERT(result == 5, "dice_roll_notation('-5+10') returns 5");
    
    // Test complex dice expressions
    result = dice_roll_notation("2d6+1d4");
    TEST_ASSERT(result >= 3 && result <= 16, "dice_roll_notation('2d6+1d4') returns value between 3 and 16");
    
    // Test implicit count
    result = dice_roll_notation("d6");
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll_notation('d6') returns value between 1 and 6");
    
    // Test invalid inputs (negative tests)
    result = dice_roll_notation(NULL);
    TEST_ASSERT(result == -1, "dice_roll_notation(NULL) returns -1");
    
    result = dice_roll_notation("invalid");
    TEST_ASSERT(result == -1, "dice_roll_notation('invalid') returns -1");
    
    result = dice_roll_notation("10/0");
    TEST_ASSERT(result == -1, "dice_roll_notation('10/0') returns -1 (division by zero)");
    
    result = dice_roll_notation("");
    TEST_ASSERT(result == -1, "dice_roll_notation('') returns -1");
    
    result = dice_roll_notation("0d6");
    TEST_ASSERT(result == -1, "dice_roll_notation('0d6') returns -1");
    
    result = dice_roll_notation("1d0");
    TEST_ASSERT(result == -1, "dice_roll_notation('1d0') returns -1");
    
    return 1;
}

// =============================================================================
// Uniformity Tests - Statistical Analysis
// =============================================================================

int test_dice_uniformity() {
    
    // Test 6-sided die uniformity using context-based API for proper seeding
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return 0;
    
    // Set a fixed seed for reproducible but varied results
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    int frequencies[6] = {0};
    int total_rolls = UNIFORMITY_SAMPLE_SIZE;
    
    for (int i = 0; i < total_rolls; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, 6);
        if (roll >= 1 && roll <= 6) {
            frequencies[roll - 1]++;
        }
    }
    
    dice_context_destroy(ctx);
    
    // Expected frequency for uniform distribution
    int expected = total_rolls / 6;
    
    // Chi-square test for uniformity
    double chi_square = chi_square_test(frequencies, expected, 6);
    
    printf("Chi-square statistic: %f (critical value: %f)\n", chi_square, CHI_SQUARE_CRITICAL_95);
    
    // For 6 categories (6-sided die), degrees of freedom = 5
    // Using chi-square critical value for 95% confidence: 11.07
    TEST_ASSERT(chi_square < 11.07, "6-sided die shows uniform distribution");
    
    // Test that no single outcome is extremely over or under-represented
    for (int i = 0; i < 6; i++) {
        double deviation = abs(frequencies[i] - expected) / (double)expected;
        TEST_ASSERT(deviation < 0.15, "No single die face is over/under-represented by more than 15%");
    }
    
    return 1;
}

int test_multiple_dice_uniformity() {
    
    // Test that multiple dice rolls have appropriate distribution using context-based API
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return 0;
    
    // Set a fixed seed for reproducible but varied results
    dice_rng_vtable_t rng = dice_create_system_rng(54321);
    dice_context_set_rng(ctx, &rng);
    
    // For 2d6, results range from 2 to 12 with bell curve distribution
    int frequencies[11] = {0}; // Index 0 = sum of 2, Index 10 = sum of 12
    int total_rolls = UNIFORMITY_SAMPLE_SIZE;
    
    for (int i = 0; i < total_rolls; i++) {
        int roll1 = ctx->rng.roll(ctx->rng.state, 6);
        int roll2 = ctx->rng.roll(ctx->rng.state, 6);
        int sum = roll1 + roll2;
        if (sum >= 2 && sum <= 12) {
            frequencies[sum - 2]++;
        }
    }
    
    dice_context_destroy(ctx);
    
    // For 2d6, the most common result should be 7, least common should be 2 and 12
    int sum_7_freq = frequencies[5]; // 7 - 2 = 5
    int sum_2_freq = frequencies[0]; // 2 - 2 = 0
    int sum_12_freq = frequencies[10]; // 12 - 2 = 10
    
    TEST_ASSERT(sum_7_freq > sum_2_freq, "Sum of 7 is more common than sum of 2 in 2d6");
    TEST_ASSERT(sum_7_freq > sum_12_freq, "Sum of 7 is more common than sum of 12 in 2d6");
    // Allow some tolerance for the probabilistic test
    int diff = abs(sum_2_freq - sum_12_freq);
    double tolerance = 0.1 * sum_2_freq; // 10% tolerance
    TEST_ASSERT(diff <= tolerance || abs(sum_2_freq - sum_12_freq) <= 100, "Sum of 2 and 12 have similar probability in 2d6");
    
    return 1;
}

// =============================================================================
// Limit Tests
// =============================================================================

int test_dice_limits() {
    
    // Test maximum reasonable values work
    int result = dice_roll(1000);
    TEST_ASSERT(result >= 1 && result <= 1000, "dice_roll(1000) works correctly");
    
    result = dice_roll_multiple(100, 6);
    TEST_ASSERT(result >= 100 && result <= 600, "dice_roll_multiple(100, 6) works correctly");
    
    // Test very large values
    result = dice_roll(1000000);
    TEST_ASSERT(result >= 1 && result <= 1000000, "dice_roll() handles large side counts");
    
    // Test integer overflow protection in multiple rolls
    result = dice_roll_multiple(1000, 1000);
    TEST_ASSERT(result > 0, "Large multiple rolls don't cause integer overflow");
    
    return 1;
}

int main() {
    printf("Running core dice operation tests...\n\n");
    
    RUN_TEST(test_dice_version);
    RUN_TEST(test_dice_roll);
    RUN_TEST(test_dice_roll_multiple);
    RUN_TEST(test_dice_roll_individual);
    RUN_TEST(test_dice_roll_notation);
    RUN_TEST(test_dice_uniformity);
    RUN_TEST(test_multiple_dice_uniformity);
    RUN_TEST(test_dice_limits);
    
    printf("All core dice tests passed!\n");
    
    return 0;
}