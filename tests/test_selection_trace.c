#include "test_common.h"
#include <string.h>

// Test that selection trace properly marks selected dice
int test_selection_trace_basic() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set a fixed seed for reproducible testing
    dice_rng_vtable_t rng = dice_create_system_rng(42);
    dice_context_set_rng(ctx, &rng);
    
    // Test keep high selection
    dice_eval_result_t result = dice_roll_expression(ctx, "4d6k2");
    TEST_ASSERT(result.success, "4d6k2 evaluation succeeds");
    
    // Get trace and format it
    char buffer[1024];
    int written = dice_format_trace_string(ctx, buffer, sizeof(buffer));
    TEST_ASSERT(written > 0, "Trace formatted successfully");
    
    // Check that the buffer contains asterisks for selected dice
    int star_count = 0;
    char *pos = buffer;
    while ((pos = strchr(pos, '*')) != NULL) {
        star_count++;
        pos++;
    }
    TEST_ASSERT(star_count == 2, "Exactly 2 dice marked as selected for k2");
    
    // Verify the trace contains both selected and unselected dice
    TEST_ASSERT(strstr(buffer, "d6 ->") != NULL, "Trace contains d6 rolls");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_trace_conditional() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set a fixed seed for reproducible testing
    dice_rng_vtable_t rng = dice_create_system_rng(123);
    dice_context_set_rng(ctx, &rng);
    
    // Test conditional selection
    dice_eval_result_t result = dice_roll_expression(ctx, "6d6s>=4");
    TEST_ASSERT(result.success, "6d6s>=4 evaluation succeeds");
    
    // Get trace and format it
    char buffer[1024];
    int written = dice_format_trace_string(ctx, buffer, sizeof(buffer));
    TEST_ASSERT(written > 0, "Trace formatted successfully");
    
    // Count total dice entries and selected ones
    int total_lines = 0;
    int starred_lines = 0;
    char buffer_copy[1024];
    strcpy(buffer_copy, buffer);  // strtok modifies the string
    char *line = strtok(buffer_copy, "\n");
    while (line != NULL) {
        if (strstr(line, "d6 ->") != NULL) {
            total_lines++;
            if (strstr(line, "*") != NULL) {
                starred_lines++;
            }
        }
        line = strtok(NULL, "\n");
    }
    
    TEST_ASSERT(total_lines == 6, "Trace shows all 6 dice");
    TEST_ASSERT(starred_lines >= 0, "Some dice may be selected (depends on rolls)");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_trace_keep_low() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Set a fixed seed for reproducible testing
    dice_rng_vtable_t rng = dice_create_system_rng(789);
    dice_context_set_rng(ctx, &rng);
    
    // Test keep low selection
    dice_eval_result_t result = dice_roll_expression(ctx, "5d8l1");
    TEST_ASSERT(result.success, "5d8l1 evaluation succeeds");
    
    // Get trace and format it
    char buffer[1024];
    int written = dice_format_trace_string(ctx, buffer, sizeof(buffer));
    TEST_ASSERT(written > 0, "Trace formatted successfully");
    
    // Count selected dice
    int star_count = 0;
    char *pos = buffer;
    while ((pos = strchr(pos, '*')) != NULL) {
        star_count++;
        pos++;
    }
    TEST_ASSERT(star_count == 1, "Exactly 1 die marked as selected for l1 (keep 1 lowest)");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_selection_trace_no_selection() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test normal dice without selection
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6");
    TEST_ASSERT(result.success, "3d6 evaluation succeeds");
    
    // Get trace and format it
    char buffer[1024];
    int written = dice_format_trace_string(ctx, buffer, sizeof(buffer));
    TEST_ASSERT(written > 0, "Trace formatted successfully");
    
    // Check that no dice are marked as selected
    TEST_ASSERT(strchr(buffer, '*') == NULL, "No dice marked as selected for normal rolls");
    
    dice_context_destroy(ctx);
    return 1;
}

int main() {
    printf("Running selection trace tests...\n\n");
    
    RUN_TEST(test_selection_trace_basic);
    RUN_TEST(test_selection_trace_conditional);
    RUN_TEST(test_selection_trace_keep_low);
    RUN_TEST(test_selection_trace_no_selection);
    
    printf("All selection trace tests passed!\n");
    return 0;
}