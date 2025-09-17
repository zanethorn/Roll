#include "test_common.h"

// =============================================================================
// AST Visitor API Tests
// =============================================================================

// Test data for visitor callback tracking
typedef struct {
    int enter_count;
    int exit_count;
    int literal_count;
    int binary_op_count;
    int dice_op_count;
    int function_call_count;
    int annotation_count;
} visitor_test_data_t;

// Test visitor callbacks
static void test_enter_node(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->enter_count++;
}

static void test_exit_node(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->exit_count++;
}

static void test_visit_literal(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->literal_count++;
    // Verify node type is correct (would crash if wrong, indicating test failure)
    if (node->type != DICE_NODE_LITERAL) {
        printf("FAIL: Literal visitor received wrong node type\n");
        exit(1);
    }
}

static void test_visit_binary_op(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->binary_op_count++;
    if (node->type != DICE_NODE_BINARY_OP) {
        printf("FAIL: Binary op visitor received wrong node type\n");
        exit(1);
    }
}

static void test_visit_dice_op(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->dice_op_count++;
    if (node->type != DICE_NODE_DICE_OP) {
        printf("FAIL: Dice op visitor received wrong node type\n");
        exit(1);
    }
}

static void test_visit_function_call(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->function_call_count++;
    if (node->type != DICE_NODE_FUNCTION_CALL) {
        printf("FAIL: Function call visitor received wrong node type\n");
        exit(1);
    }
}

static void test_visit_annotation(const dice_ast_node_t *node, void *user_data) {
    visitor_test_data_t *data = (visitor_test_data_t *)user_data;
    data->annotation_count++;
    if (node->type != DICE_NODE_ANNOTATION) {
        printf("FAIL: Annotation visitor received wrong node type\n");
        exit(1);
    }
}

int test_visitor_basic() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    TEST_ASSERT(ctx != NULL, "dice_context_create() returns non-null");
    
    // Parse a simple literal expression
    dice_ast_node_t *ast = dice_parse(ctx, "42");
    TEST_ASSERT(ast != NULL, "dice_parse('42') returns non-null AST");
    TEST_ASSERT(!dice_has_error(ctx), "No error after parsing literal");
    
    // Set up visitor
    visitor_test_data_t test_data = {0};
    dice_ast_visitor_t visitor = {0};
    visitor.enter_node = test_enter_node;
    visitor.exit_node = test_exit_node;
    visitor.visit_literal = test_visit_literal;
    visitor.user_data = &test_data;
    
    // Traverse AST
    dice_ast_traverse(ast, &visitor);
    
    // Verify visitor was called correctly
    TEST_ASSERT(test_data.enter_count == 1, "Enter node called once for literal");
    TEST_ASSERT(test_data.exit_count == 1, "Exit node called once for literal");
    TEST_ASSERT(test_data.literal_count == 1, "Literal visitor called once");
    TEST_ASSERT(test_data.binary_op_count == 0, "Binary op visitor not called");
    TEST_ASSERT(test_data.dice_op_count == 0, "Dice op visitor not called");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_visitor_complex_expression() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Parse complex expression "3d6+2"
    dice_ast_node_t *ast = dice_parse(ctx, "3d6+2");
    TEST_ASSERT(ast != NULL, "dice_parse('3d6+2') returns non-null AST");
    TEST_ASSERT(!dice_has_error(ctx), "No error after parsing complex expression");
    
    // Set up visitor
    visitor_test_data_t test_data = {0};
    dice_ast_visitor_t visitor = {0};
    visitor.enter_node = test_enter_node;
    visitor.exit_node = test_exit_node;
    visitor.visit_literal = test_visit_literal;
    visitor.visit_binary_op = test_visit_binary_op;
    visitor.visit_dice_op = test_visit_dice_op;
    visitor.user_data = &test_data;
    
    // Traverse AST
    dice_ast_traverse(ast, &visitor);
    
    // Verify visitor was called correctly for complex expression
    // Should have: binary_op(+), dice_op(3d6), literal(3), literal(6), literal(2)
    TEST_ASSERT(test_data.enter_count >= 4, "Enter node called multiple times for complex expression");
    TEST_ASSERT(test_data.exit_count == test_data.enter_count, "Exit node called same number of times as enter");
    TEST_ASSERT(test_data.literal_count >= 3, "Multiple literals visited in complex expression");
    TEST_ASSERT(test_data.binary_op_count == 1, "One binary op visited in 3d6+2");
    TEST_ASSERT(test_data.dice_op_count == 1, "One dice op visited in 3d6+2");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_visitor_null_safety() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    dice_ast_node_t *ast = dice_parse(ctx, "1");
    
    // Test null node
    dice_ast_visitor_t visitor = {0};
    visitor.enter_node = test_enter_node;
    visitor.user_data = NULL;
    
    dice_ast_traverse(NULL, &visitor); // Should not crash
    dice_ast_traverse(ast, NULL);       // Should not crash
    
    // Test with some visitor callbacks null
    visitor_test_data_t test_data = {0};
    visitor.user_data = &test_data;
    visitor.exit_node = NULL;     // Some callbacks null
    visitor.visit_literal = NULL;
    
    dice_ast_traverse(ast, &visitor);
    
    // Should still call enter_node
    TEST_ASSERT(test_data.enter_count == 1, "Enter node called even when other callbacks are null");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_trace_visitor() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Parse expression to test trace visitor
    dice_ast_node_t *ast = dice_parse(ctx, "2+3");
    TEST_ASSERT(ast != NULL, "dice_parse('2+3') returns non-null AST");
    
    // Create trace visitor (output to stdout for testing)
    dice_ast_visitor_t trace_visitor = dice_create_trace_visitor(stdout, "  ");
    
    printf("=== Testing trace visitor output for '2+3' ===\n");
    // This will print the AST structure to stdout
    dice_ast_traverse(ast, &trace_visitor);
    printf("=== End trace visitor output ===\n");
    
    // Test creating trace visitor with different parameters
    dice_ast_visitor_t trace_visitor2 = dice_create_trace_visitor(stderr, "\t");
    TEST_ASSERT(trace_visitor2.enter_node != NULL, "Trace visitor has enter_node callback");
    TEST_ASSERT(trace_visitor2.exit_node != NULL, "Trace visitor has exit_node callback");
    TEST_ASSERT(trace_visitor2.visit_literal != NULL, "Trace visitor has visit_literal callback");
    TEST_ASSERT(trace_visitor2.visit_binary_op != NULL, "Trace visitor has visit_binary_op callback");
    TEST_ASSERT(trace_visitor2.user_data != NULL, "Trace visitor has user_data");
    
    dice_context_destroy(ctx);
    return 1;
}

int test_dice_expression_visitor() {
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    
    // Parse a dice expression
    dice_ast_node_t *ast = dice_parse(ctx, "4d6");
    TEST_ASSERT(ast != NULL, "dice_parse('4d6') returns non-null AST");
    
    // Test trace visitor with dice expression
    dice_ast_visitor_t trace_visitor = dice_create_trace_visitor(stdout, "| ");
    
    printf("\n=== Testing trace visitor output for '4d6' ===\n");
    dice_ast_traverse(ast, &trace_visitor);
    printf("=== End trace visitor output ===\n");
    
    dice_context_destroy(ctx);
    return 1;
}

// =============================================================================
// Test Runner
// =============================================================================

int main() {
    printf("Running AST Visitor tests...\n\n");
    
    RUN_TEST(test_visitor_basic);
    RUN_TEST(test_visitor_complex_expression);
    RUN_TEST(test_visitor_null_safety);
    RUN_TEST(test_trace_visitor);
    RUN_TEST(test_dice_expression_visitor);
    
    printf("All AST Visitor tests passed!\n");
    return 0;
}