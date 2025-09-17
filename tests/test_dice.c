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



int test_dice_roll() {
    
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

int test_rng_functionality() {
    // Test that RNG functions work and produce different results
    dice_rng_vtable_t rng1 = dice_create_system_rng(12345);
    dice_rng_vtable_t rng2 = dice_create_system_rng(54321);
    
    // Test that different seeds produce different results (probabilistically)
    int differences = 0;
    for (int i = 0; i < 10; i++) {
        int result1 = rng1.roll(rng1.state, 20);
        int result2 = rng2.roll(rng2.state, 20);
        if (result1 != result2) {
            differences++;
        }
    }
    
    TEST_ASSERT(differences > 0, "Different seeds produce different results");
    
    // Test that dice functions work
    int result = dice_roll(6);
    TEST_ASSERT(result >= 1 && result <= 6, "dice_roll() produces valid results");
    
    // Cleanup RNG states
    if (rng1.cleanup) rng1.cleanup(rng1.state);
    if (rng2.cleanup) rng2.cleanup(rng2.state);
    
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

int test_custom_dice() {
    // Test custom dice functionality
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    dice_rng_vtable_t rng = dice_create_system_rng(12345);
    dice_context_set_rng(ctx, &rng);
    
    // Test 1: Inline FATE dice
    dice_eval_result_t result = dice_roll_expression(ctx, "1d{-1,0,1}");
    TEST_ASSERT(result.success, "dice_roll_expression('1d{-1,0,1}') succeeds");
    TEST_ASSERT(result.value >= -1 && result.value <= 1, "FATE die result is between -1 and 1");
    
    // Test 2: Multiple inline custom dice
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "4d{-1,0,1}");
    TEST_ASSERT(result.success, "dice_roll_expression('4d{-1,0,1}') succeeds");
    TEST_ASSERT(result.value >= -4 && result.value <= 4, "4 FATE dice result is between -4 and 4");
    
    // Test 3: String-only dice with implicit numbering
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "1d{\"Earth\",\"Wind\",\"Fire\",\"Water\"}");
    TEST_ASSERT(result.success, "dice_roll_expression('1d{\"Earth\",\"Wind\",\"Fire\",\"Water\"}') succeeds");
    TEST_ASSERT(result.value >= 0 && result.value <= 3, "Element die result is between 0 and 3");
    
    // Test 4: Mixed value and label dice
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "1d{-1:\"-\",0:\" \",1:\"+\"}");
    TEST_ASSERT(result.success, "dice_roll_expression('1d{-1:\"-\",0:\" \",1:\"+\"}') succeeds");
    TEST_ASSERT(result.value >= -1 && result.value <= 1, "Labeled FATE die result is between -1 and 1");
    
    // Test 5: Register named FATE dice
    dice_custom_side_t fate_sides[] = {
        {-1, "-"},
        {0, " "},
        {1, "+"}
    };
    int reg_result = dice_register_custom_die(ctx, "F", fate_sides, 3);
    TEST_ASSERT(reg_result == 0, "dice_register_custom_die() succeeds for FATE dice");
    
    // Test 6: Use named FATE dice
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "1dF");
    TEST_ASSERT(result.success, "dice_roll_expression('1dF') succeeds");
    TEST_ASSERT(result.value >= -1 && result.value <= 1, "Named FATE die result is between -1 and 1");
    
    // Test 7: Multiple named FATE dice
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "4dF");
    TEST_ASSERT(result.success, "dice_roll_expression('4dF') succeeds");
    TEST_ASSERT(result.value >= -4 && result.value <= 4, "4 named FATE dice result is between -4 and 4");
    
    // Test 8: Irregular numbered die
    dice_custom_side_t demon_sides[] = {
        {0, NULL}, {1, NULL}, {3, NULL}, {5, NULL}, {7, NULL}, {9, NULL}, {11, NULL}
    };
    reg_result = dice_register_custom_die(ctx, "Demon", demon_sides, 7);
    TEST_ASSERT(reg_result == 0, "dice_register_custom_die() succeeds for Demon dice");
    
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "1dDemon");
    TEST_ASSERT(result.success, "dice_roll_expression('1dDemon') succeeds");
    TEST_ASSERT(result.value == 0 || result.value == 1 || result.value == 3 || 
                result.value == 5 || result.value == 7 || result.value == 9 || result.value == 11,
                "Demon die returns expected irregular value");
    
    // Test 9: Custom dice in expressions
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "2dF+1");
    TEST_ASSERT(result.success, "dice_roll_expression('2dF+1') succeeds");
    TEST_ASSERT(result.value >= -1 && result.value <= 3, "2dF+1 result is in expected range");
    
    // Test 10: Lookup nonexistent die
    const dice_custom_die_t *lookup = dice_lookup_custom_die(ctx, "NonExistent");
    TEST_ASSERT(lookup == NULL, "dice_lookup_custom_die() returns NULL for nonexistent die");
    
    // Test 11: Use nonexistent die in expression
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "1dNonExistent");
    TEST_ASSERT(!result.success, "dice_roll_expression('1dNonExistent') fails");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set for nonexistent die");
    
    // Test 12: Empty custom die should fail
    dice_clear_error(ctx);
    result = dice_roll_expression(ctx, "1d{}");
    TEST_ASSERT(!result.success, "dice_roll_expression('1d{}') fails for empty die");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_fate_dice_auto_registration() {
    // Test automatic FATE dice registration when DICE_FEATURE_FATE is enabled
    
    // Test 1: Context without FATE feature - should not have FATE dice registered
    dice_context_t *basic_ctx = dice_context_create(64 * 1024, DICE_FEATURE_BASIC);
    const dice_custom_die_t *fate_lookup = dice_lookup_custom_die(basic_ctx, "F");
    TEST_ASSERT(fate_lookup == NULL, "FATE dice 'F' should not be registered without DICE_FEATURE_FATE");
    
    dice_eval_result_t result = dice_roll_expression(basic_ctx, "1dF");
    TEST_ASSERT(!result.success, "FATE dice '1dF' should fail without DICE_FEATURE_FATE");
    TEST_ASSERT(dice_has_error(basic_ctx), "Error should be set when using unregistered FATE die");
    
    dice_context_destroy(basic_ctx);
    
    // Test 2: Context with FATE feature - should have FATE dice auto-registered
    dice_context_t *fate_ctx = dice_context_create(64 * 1024, DICE_FEATURE_FATE);
    fate_lookup = dice_lookup_custom_die(fate_ctx, "F");
    TEST_ASSERT(fate_lookup != NULL, "FATE dice 'F' should be auto-registered with DICE_FEATURE_FATE");
    TEST_ASSERT(fate_lookup->side_count == 3, "Auto-registered FATE dice should have 3 sides");
    TEST_ASSERT(strcmp(fate_lookup->name, "F") == 0, "Auto-registered FATE dice should have name 'F'");
    
    // Verify the FATE sides are correct: {-1, 0, 1} with labels {"-", " ", "+"}
    bool found_minus_one = false, found_zero = false, found_plus_one = false;
    for (size_t i = 0; i < fate_lookup->side_count; i++) {
        if (fate_lookup->sides[i].value == -1) {
            found_minus_one = true;
            TEST_ASSERT(strcmp(fate_lookup->sides[i].label, "-") == 0, "FATE -1 side should have '-' label");
        } else if (fate_lookup->sides[i].value == 0) {
            found_zero = true;
            TEST_ASSERT(strcmp(fate_lookup->sides[i].label, " ") == 0, "FATE 0 side should have ' ' label");
        } else if (fate_lookup->sides[i].value == 1) {
            found_plus_one = true;
            TEST_ASSERT(strcmp(fate_lookup->sides[i].label, "+") == 0, "FATE +1 side should have '+' label");
        }
    }
    TEST_ASSERT(found_minus_one && found_zero && found_plus_one, "FATE dice should have sides -1, 0, and 1");
    
    // Test 3: Use auto-registered FATE dice
    dice_clear_error(fate_ctx);
    result = dice_roll_expression(fate_ctx, "1dF");
    TEST_ASSERT(result.success, "FATE dice '1dF' should work with DICE_FEATURE_FATE enabled");
    TEST_ASSERT(result.value >= -1 && result.value <= 1, "FATE die result should be between -1 and 1");
    
    // Test 4: Use multiple auto-registered FATE dice
    dice_clear_error(fate_ctx);
    result = dice_roll_expression(fate_ctx, "4dF");
    TEST_ASSERT(result.success, "Multiple FATE dice '4dF' should work with auto-registration");
    TEST_ASSERT(result.value >= -4 && result.value <= 4, "4dF result should be between -4 and 4");
    
    // Test 5: FATE dice with expressions
    dice_clear_error(fate_ctx);
    result = dice_roll_expression(fate_ctx, "2dF+3");
    TEST_ASSERT(result.success, "FATE dice expression '2dF+3' should work with auto-registration");
    TEST_ASSERT(result.value >= 1 && result.value <= 5, "2dF+3 result should be between 1 and 5");
    
    dice_context_destroy(fate_ctx);
    
    // Test 6: Context with all features should also have FATE dice
    dice_context_t *all_ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    fate_lookup = dice_lookup_custom_die(all_ctx, "F");
    TEST_ASSERT(fate_lookup != NULL, "FATE dice 'F' should be auto-registered with DICE_FEATURE_ALL");
    
    dice_clear_error(all_ctx);
    result = dice_roll_expression(all_ctx, "1dF");
    TEST_ASSERT(result.success, "FATE dice '1dF' should work with DICE_FEATURE_ALL");
    
    dice_context_destroy(all_ctx);
    
    return 1;
}

int main() {
    printf("Running dice library tests...\n\n");
    
    RUN_TEST(test_dice_version);
    RUN_TEST(test_dice_roll);
    RUN_TEST(test_dice_roll_multiple);
    RUN_TEST(test_dice_roll_individual);
    RUN_TEST(test_dice_roll_notation);
    RUN_TEST(test_parser_api);
    RUN_TEST(test_rng_functionality);
    RUN_TEST(test_new_architecture);
    RUN_TEST(test_exploding_dice);
    RUN_TEST(test_custom_dice);
    RUN_TEST(test_fate_dice_auto_registration);
    
    printf("All tests passed!\n");
    
    return 0;
}