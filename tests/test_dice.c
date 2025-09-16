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
    
    // Test invalid notation
    result = dice_roll_notation(NULL);
    TEST_ASSERT(result == -1, "dice_roll_notation(NULL) returns -1");
    
    result = dice_roll_notation("invalid");
    TEST_ASSERT(result == -1, "dice_roll_notation('invalid') returns -1");
    
    result = dice_roll_notation("d6");
    TEST_ASSERT(result == -1, "dice_roll_notation('d6') returns -1");
    
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
    
    printf("All tests passed!\n");
    return 0;
}