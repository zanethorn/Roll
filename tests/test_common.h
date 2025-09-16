#ifndef TEST_COMMON_H
#define TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <time.h>
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

// Statistical test helpers
#define CHI_SQUARE_CRITICAL_95 7.815  // For 3 degrees of freedom, 95% confidence
#define UNIFORMITY_SAMPLE_SIZE 10000

// Memory test helpers
#define SMALL_ARENA_SIZE (1024)
#define LARGE_ARENA_SIZE (1024 * 1024)

// Test utilities
static inline double chi_square_test(int* observed, int expected, int categories) {
    double chi_square = 0.0;
    for (int i = 0; i < categories; i++) {
        double diff = observed[i] - expected;
        chi_square += (diff * diff) / expected;
    }
    return chi_square;
}

#endif /* TEST_COMMON_H */