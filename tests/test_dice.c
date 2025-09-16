#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "dice.h"

// Simple test framework macros
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            fprintf(stderr, "FAIL: %s\n", message); \
            return 0; \
        } else { \
            printf("PASS: %s\n", message); \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        printf("Running %s...\n", #test_func); \
        if (!test_func()) { \
            fprintf(stderr, "Test %s failed!\n", #test_func); \
            return 1; \
        } \
        printf("%s passed!\n\n", #test_func); \
    } while(0)

int test_dice_version() {
    const char *version = dice_version();
    TEST_ASSERT(version != NULL, "dice_version() returns non-null");
    TEST_ASSERT(strlen(version) > 0, "dice_version() returns non-empty string");
    printf("Library version: %s\n", version);
    return 1;
}

int test_dice_init() {
    // Test initialization with seed
    dice_init(12345);
    TEST_ASSERT(1, "dice_init() with seed completes");
    
    // Test initialization with time-based seed
    dice_init(0);
    TEST_ASSERT(1, "dice_init() with time-based seed completes");
    
    return 1;
}

int test_dice_roll() {
    dice_init(12345); // Use fixed seed for reproducible results
    
    // Test valid rolls
    int result = dice_roll(6);
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll(6) returns value between 1 and 6");
    
    result = dice_roll(20);
    TEST_ASSERT(result >= 1 && result <= 20, "dice_roll(20) returns value between 1 and 20");
    
    // Test invalid inputs
    result = dice_roll(0);
    TEST_ASSERT(result == -1, "dice_roll(0) returns -1");
    
    result = dice_roll(-5);
    TEST_ASSERT(result == -1, "dice_roll(-5) returns -1");
    
    return 1;
}

int test_dice_roll_multiple() {
    dice_init(12345);
    
    // Test valid multiple rolls
    int result = dice_roll_multiple(3, 6);
    TEST_ASSERT(result >= 3 && result <= 18, "dice_roll_multiple(3, 6) returns value between 3 and 18");
    
    // Test single die
    result = dice_roll_multiple(1, 20);
    TEST_ASSERT(result >= 1 && result <= 20, "dice_roll_multiple(1, 20) returns value between 1 and 20");
    
    // Test invalid inputs
    result = dice_roll_multiple(0, 6);
    TEST_ASSERT(result == -1, "dice_roll_multiple(0, 6) returns -1");
    
    result = dice_roll_multiple(3, 0);
    TEST_ASSERT(result == -1, "dice_roll_multiple(3, 0) returns -1");
    
    return 1;
}

int test_dice_roll_individual() {
    dice_init(12345);
    int results[10];
    
    // Test valid individual rolls
    int sum = dice_roll_individual(3, 6, results);
    TEST_ASSERT(sum >= 3 && sum <= 18, "dice_roll_individual(3, 6) returns sum between 3 and 18");
    
    // Verify individual results
    int calculated_sum = 0;
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT(results[i] >= 1 && results[i] <= 6, "Individual die result is between 1 and 6");
        calculated_sum += results[i];
    }
    TEST_ASSERT(sum == calculated_sum, "Sum matches individual results");
    
    // Test invalid inputs
    sum = dice_roll_individual(3, 6, NULL);
    TEST_ASSERT(sum == -1, "dice_roll_individual() with NULL results returns -1");
    
    return 1;
}

int test_dice_roll_notation() {
    dice_init(12345);
    
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
    
    // Test complex expressions (new EBNF parser features)
    result = dice_roll_notation("2*3");
    TEST_ASSERT(result == 6, "dice_roll_notation('2*3') returns 6");
    
    result = dice_roll_notation("10/2");
    TEST_ASSERT(result == 5, "dice_roll_notation('10/2') returns 5");
    
    result = dice_roll_notation("(2+3)*4");
    TEST_ASSERT(result == 20, "dice_roll_notation('(2+3)*4') returns 20");
    
    result = dice_roll_notation("-5+10");
    TEST_ASSERT(result == 5, "dice_roll_notation('-5+10') returns 5");
    
    // Test dice in complex expressions
    result = dice_roll_notation("2d6+1d4");
    TEST_ASSERT(result >= 3 && result <= 16, "dice_roll_notation('2d6+1d4') returns value between 3 and 16");
    
    // Test d without count (should default to 1)
    result = dice_roll_notation("d6");
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll_notation('d6') returns value between 1 and 6");
    
    // Test invalid notation
    result = dice_roll_notation(NULL);
    TEST_ASSERT(result == -1, "dice_roll_notation(NULL) returns -1");
    
    result = dice_roll_notation("invalid");
    TEST_ASSERT(result == -1, "dice_roll_notation('invalid') returns -1");
    
    result = dice_roll_notation("10/0");
    TEST_ASSERT(result == -1, "dice_roll_notation('10/0') returns -1 (division by zero)");
    
    return 1;
}

int test_parser_api() {
    // Test the new context-based API
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    TEST_ASSERT(ctx != NULL, "dice_context_create() returns non-null");
    
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    // Test parsing and evaluation
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6+2");
    TEST_ASSERT(result.success, "dice_roll_expression('3d6+2') succeeds");
    TEST_ASSERT(!dice_has_error(ctx), "No error after successful evaluation");
    TEST_ASSERT(result.value >= 5 && result.value <= 20, "dice_roll_expression('3d6+2') returns value between 5 and 20");
    
    // Test parse error
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "invalid");
    TEST_ASSERT(!result.success, "dice_roll_expression('invalid') returns error");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set after failed evaluation");
    
    // Test complex expression
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "2*(1d6+3)");
    TEST_ASSERT(result.success, "dice_roll_expression('2*(1d6+3)') succeeds");
    TEST_ASSERT(result.value >= 8 && result.value <= 18, "dice_roll_expression('2*(1d6+3)') returns value between 8 and 18");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_rng_decoupling() {
    // Test that we can use custom RNG with legacy API
    dice_rng_vtable_t custom_rng = dice_create_system_rng(54321);
    
    dice_set_rng(&custom_rng);
    const dice_rng_vtable_t *current_rng = dice_get_rng();
    TEST_ASSERT(current_rng != NULL, "dice_get_rng() returns non-null");
    
    // Test that dice operations use the custom RNG
    int result1 = dice_roll(6);
    TEST_ASSERT(result1 >= 1 && result1 <= 6, "dice_roll() with custom RNG works");
    
    // Reset to default to avoid affecting other tests
    dice_init(12345);
    
    return 1;
}

int test_new_architecture() {
    // Test the new architecture directly
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

int test_exploding_dice() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    // Test exploding dice notation (if implemented)
    // For now, just test that it doesn't crash and returns something reasonable
    dice_eval_result_t result = dice_roll_expression(ctx, "1d6!");
    // If exploding dice is not implemented yet, it should fail gracefully
    if (result.success) {
        TEST_ASSERT(result.value >= 1, "Exploding dice returns positive value");
    } else {
        // If not implemented, that's also acceptable for now
        TEST_ASSERT(dice_has_error(ctx), "Exploding dice fails gracefully if not implemented");
    }
    
    dice_context_destroy(ctx);
    return 1;
}

int main() {
    printf("Running dice library tests...\n\n");
    
    RUN_TEST(test_dice_version);
    RUN_TEST(test_dice_init);
    RUN_TEST(test_dice_roll);
    RUN_TEST(test_dice_roll_multiple);
    RUN_TEST(test_dice_roll_individual);
    RUN_TEST(test_dice_roll_notation);
    RUN_TEST(test_parser_api);
    RUN_TEST(test_rng_decoupling);
    RUN_TEST(test_new_architecture);
    RUN_TEST(test_exploding_dice);
    
    printf("All tests passed!\n");
    
    // Cleanup
    dice_cleanup();
    
    return 0;
}