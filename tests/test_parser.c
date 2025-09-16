#include "test_common.h"

// =============================================================================
// Parser API Tests
// =============================================================================

int test_parser_basic() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    TEST_ASSERT(ctx != NULL, "dice_context_create() returns non-null");
    
    // Test basic expression parsing
    dice_eval_result_t result = dice_roll_expression(ctx, "3d6+2");
    TEST_ASSERT(result.success, "dice_roll_expression('3d6+2') succeeds");
    TEST_ASSERT(!dice_has_error(ctx), "No error after successful evaluation");
    TEST_ASSERT(result.value >= 5 && result.value <= 20, "dice_roll_expression('3d6+2') returns value between 5 and 20");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_error_handling() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test invalid expression
    dice_eval_result_t result = dice_roll_expression(ctx, "invalid");
    TEST_ASSERT(!result.success, "dice_roll_expression('invalid') returns error");
    TEST_ASSERT(dice_has_error(ctx), "Error flag set after failed evaluation");
    
    const char* error_msg = dice_get_error(ctx);
    TEST_ASSERT(error_msg != NULL, "Error message is available");
    TEST_ASSERT(strlen(error_msg) > 0, "Error message is not empty");
    
    // Clear error and verify
    dice_clear_error(ctx);
    TEST_ASSERT(!dice_has_error(ctx), "Error cleared after dice_clear_error()");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_complex_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test complex expression with parentheses
    dice_eval_result_t result = dice_roll_expression(ctx, "2*(1d6+3)");
    TEST_ASSERT(result.success, "dice_roll_expression('2*(1d6+3)') succeeds");
    TEST_ASSERT(result.value >= 8 && result.value <= 18, "dice_roll_expression('2*(1d6+3)') returns value between 8 and 18");
    
    // Test nested dice expressions
    result = dice_roll_expression(ctx, "(2d6)+(1d4+2)");
    TEST_ASSERT(result.success, "dice_roll_expression('(2d6)+(1d4+2)') succeeds");
    TEST_ASSERT(result.value >= 5 && result.value <= 18, "Complex nested expression returns expected range");
    
    // Test multiple operations
    result = dice_roll_expression(ctx, "1d6*2+1d4-1");
    TEST_ASSERT(result.success, "dice_roll_expression('1d6*2+1d4-1') succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 15, "Multiple operations expression returns expected range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_negative_inputs() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test NULL expression
    dice_eval_result_t result = dice_roll_expression(ctx, NULL);
    TEST_ASSERT(!result.success, "dice_roll_expression(NULL) returns error");
    // Note: NULL input may or may not set error flag depending on implementation
    if (dice_has_error(ctx)) {
        dice_clear_error(ctx);
    }
    
    // Test empty expression
    result = dice_roll_expression(ctx, "");
    TEST_ASSERT(!result.success, "dice_roll_expression('') returns error");
    dice_clear_error(ctx);
    
    // Test malformed expressions
    result = dice_roll_expression(ctx, "1d");
    TEST_ASSERT(!result.success, "dice_roll_expression('1d') returns error");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "d");
    TEST_ASSERT(!result.success, "dice_roll_expression('d') returns error");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "+");
    TEST_ASSERT(!result.success, "dice_roll_expression('+') returns error");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "((");
    TEST_ASSERT(!result.success, "dice_roll_expression('((') returns error for unmatched parentheses");
    dice_clear_error(ctx);
    
    result = dice_roll_expression(ctx, "))");
    TEST_ASSERT(!result.success, "dice_roll_expression('))') returns error for unmatched parentheses");
    dice_clear_error(ctx);
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_division_by_zero() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test division by zero
    dice_eval_result_t result = dice_roll_expression(ctx, "10/0");
    TEST_ASSERT(!result.success, "dice_roll_expression('10/0') returns error");
    TEST_ASSERT(dice_has_error(ctx), "Error set for division by zero");
    
    const char* error_msg = dice_get_error(ctx);
    TEST_ASSERT(strstr(error_msg, "zero") != NULL || strstr(error_msg, "division") != NULL, 
                "Error message mentions division by zero");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_dice_notation_variations() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test various dice notation formats
    dice_eval_result_t result = dice_roll_expression(ctx, "1d6");
    TEST_ASSERT(result.success, "dice_roll_expression('1d6') succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "'1d6' returns valid range");
    
    result = dice_roll_expression(ctx, "1D6");  // Uppercase D
    TEST_ASSERT(result.success, "dice_roll_expression('1D6') succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "'1D6' returns valid range");
    
    result = dice_roll_expression(ctx, "d6");   // Implicit 1
    TEST_ASSERT(result.success, "dice_roll_expression('d6') succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "'d6' returns valid range");
    
    result = dice_roll_expression(ctx, "D6");   // Uppercase D with implicit 1
    TEST_ASSERT(result.success, "dice_roll_expression('D6') succeeds");
    TEST_ASSERT(result.value >= 1 && result.value <= 6, "'D6' returns valid range");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_whitespace_handling() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test expressions with various whitespace
    dice_eval_result_t result = dice_roll_expression(ctx, " 1d6 ");
    TEST_ASSERT(result.success, "dice_roll_expression(' 1d6 ') succeeds");
    
    result = dice_roll_expression(ctx, "1 d 6");
    TEST_ASSERT(result.success, "dice_roll_expression('1 d 6') succeeds");
    
    result = dice_roll_expression(ctx, "  2  +  3  ");
    TEST_ASSERT(result.success, "dice_roll_expression('  2  +  3  ') succeeds");
    TEST_ASSERT(result.value == 5, "Whitespace-padded arithmetic works correctly");
    
    result = dice_roll_expression(ctx, "\t1d6\n");
    TEST_ASSERT(result.success, "dice_roll_expression with tabs and newlines succeeds");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_large_expressions() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test very long but valid expression
    dice_eval_result_t result = dice_roll_expression(ctx, "1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1");
    TEST_ASSERT(result.success, "Very long arithmetic expression succeeds");
    TEST_ASSERT(result.value == 20, "Very long expression calculates correctly");
    
    // Test deep nesting
    result = dice_roll_expression(ctx, "((((((1+1)+1)+1)+1)+1)+1)");
    TEST_ASSERT(result.success, "Deeply nested expression succeeds");
    TEST_ASSERT(result.value == 7, "Deeply nested expression calculates correctly");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_parser_operator_precedence() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Test operator precedence
    dice_eval_result_t result = dice_roll_expression(ctx, "2+3*4");
    TEST_ASSERT(result.success, "dice_roll_expression('2+3*4') succeeds");
    TEST_ASSERT(result.value == 14, "Multiplication precedence over addition (2+3*4=14, not 20)");
    
    result = dice_roll_expression(ctx, "(2+3)*4");
    TEST_ASSERT(result.success, "dice_roll_expression('(2+3)*4') succeeds");
    TEST_ASSERT(result.value == 20, "Parentheses override precedence");
    
    result = dice_roll_expression(ctx, "12/3*2");
    TEST_ASSERT(result.success, "dice_roll_expression('12/3*2') succeeds");
    TEST_ASSERT(result.value == 8, "Left-to-right evaluation for same precedence (12/3*2=8)");
    
    result = dice_roll_expression(ctx, "2*3+4*5");
    TEST_ASSERT(result.success, "dice_roll_expression('2*3+4*5') succeeds");
    TEST_ASSERT(result.value == 26, "Multiple multiplications with addition");
    
    dice_context_destroy(ctx);
    return 1;
}

int main() {
    printf("Running parser tests...\n\n");
    
    RUN_TEST(test_parser_basic);
    RUN_TEST(test_parser_error_handling);
    RUN_TEST(test_parser_complex_expressions);
    RUN_TEST(test_parser_negative_inputs);
    RUN_TEST(test_parser_division_by_zero);
    RUN_TEST(test_parser_dice_notation_variations);
    RUN_TEST(test_parser_whitespace_handling);
    RUN_TEST(test_parser_large_expressions);
    RUN_TEST(test_parser_operator_precedence);
    
    printf("All parser tests passed!\n");
    return 0;
}