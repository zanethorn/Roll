#include "dice.h"
#include "internal.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// AST Visitor Implementation
// =============================================================================

void dice_ast_traverse(const dice_ast_node_t *node, const dice_ast_visitor_t *visitor) {
    if (!node || !visitor) {
        return;
    }
    
    // Call enter_node if provided
    if (visitor->enter_node) {
        visitor->enter_node(node, visitor->user_data);
    }
    
    // Call specific node visitor if provided
    switch (node->type) {
        case DICE_NODE_LITERAL:
            if (visitor->visit_literal) {
                visitor->visit_literal(node, visitor->user_data);
            }
            break;
            
        case DICE_NODE_BINARY_OP:
            if (visitor->visit_binary_op) {
                visitor->visit_binary_op(node, visitor->user_data);
            }
            // Recursively visit children
            dice_ast_traverse(node->data.binary_op.left, visitor);
            dice_ast_traverse(node->data.binary_op.right, visitor);
            break;
            
        case DICE_NODE_DICE_OP:
            if (visitor->visit_dice_op) {
                visitor->visit_dice_op(node, visitor->user_data);
            }
            // Recursively visit children if they exist
            if (node->data.dice_op.count) {
                dice_ast_traverse(node->data.dice_op.count, visitor);
            }
            if (node->data.dice_op.sides) {
                dice_ast_traverse(node->data.dice_op.sides, visitor);
            }
            if (node->data.dice_op.modifier) {
                dice_ast_traverse(node->data.dice_op.modifier, visitor);
            }
            break;
            
        case DICE_NODE_FUNCTION_CALL:
            if (visitor->visit_function_call) {
                visitor->visit_function_call(node, visitor->user_data);
            }
            // Recursively visit function arguments
            for (size_t i = 0; i < node->data.function_call.arg_count; i++) {
                dice_ast_traverse(node->data.function_call.args[i], visitor);
            }
            break;
            
        case DICE_NODE_ANNOTATION:
            if (visitor->visit_annotation) {
                visitor->visit_annotation(node, visitor->user_data);
            }
            // Recursively visit child if it exists
            if (node->data.annotation.child) {
                dice_ast_traverse(node->data.annotation.child, visitor);
            }
            break;
    }
    
    // Call exit_node if provided
    if (visitor->exit_node) {
        visitor->exit_node(node, visitor->user_data);
    }
}

// =============================================================================
// Default Trace Visitor Implementation
// =============================================================================

typedef struct {
    FILE *output;
    const char *indent_str;
    int depth;
} trace_visitor_data_t;

// Helper to get node type name
static const char* get_node_type_name(dice_node_type_t type) {
    switch (type) {
        case DICE_NODE_LITERAL: return "LITERAL";
        case DICE_NODE_BINARY_OP: return "BINARY_OP";
        case DICE_NODE_DICE_OP: return "DICE_OP";
        case DICE_NODE_FUNCTION_CALL: return "FUNCTION_CALL";
        case DICE_NODE_ANNOTATION: return "ANNOTATION";
        default: return "UNKNOWN";
    }
}

// Helper to get binary operator name
static const char* get_binary_op_name(dice_binary_op_t op) {
    switch (op) {
        case DICE_OP_ADD: return "+";
        case DICE_OP_SUB: return "-";
        case DICE_OP_MUL: return "*";
        case DICE_OP_DIV: return "/";
        default: return "?";
    }
}

// Helper to get dice type name
static const char* get_dice_type_name(dice_dice_type_t type) {
    switch (type) {
        case DICE_DICE_BASIC: return "BASIC";
        case DICE_DICE_EXPLODING: return "EXPLODING";
        case DICE_DICE_POOL: return "POOL";
        case DICE_DICE_FATE: return "FATE";
        case DICE_DICE_SELECT: return "SELECT";
        case DICE_DICE_CUSTOM: return "CUSTOM";
        default: return "UNKNOWN";
    }
}

// Print indentation
static void print_indent(FILE *output, const char *indent_str, int depth) {
    for (int i = 0; i < depth; i++) {
        fprintf(output, "%s", indent_str);
    }
}

static void trace_enter_node(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "%s {\n", get_node_type_name(node->type));
    
    data->depth++;
}

static void trace_exit_node(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    data->depth--;
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "}\n");
}

static void trace_literal(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "value: %lld\n", (long long)node->data.literal.value);
}

static void trace_binary_op(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "operator: %s\n", get_binary_op_name(node->data.binary_op.op));
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "left:\n");
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "right:\n");
}

static void trace_dice_op(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "dice_type: %s\n", get_dice_type_name(node->data.dice_op.dice_type));
    
    if (node->data.dice_op.custom_name) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "custom_name: %s\n", node->data.dice_op.custom_name);
    }
    
    if (node->data.dice_op.selection) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "selection: %s %lld (%s)\n", 
                node->data.dice_op.selection->is_drop_operation ? "drop" : "keep",
                (long long)node->data.dice_op.selection->count,
                node->data.dice_op.selection->select_high ? "high" : "low");
    }
    
    if (node->data.dice_op.count) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "count:\n");
    }
    
    if (node->data.dice_op.sides) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "sides:\n");
    }
    
    if (node->data.dice_op.modifier) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "modifier:\n");
    }
}

static void trace_function_call(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "function: %s\n", node->data.function_call.name);
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "arg_count: %zu\n", node->data.function_call.arg_count);
    
    if (node->data.function_call.arg_count > 0) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "args:\n");
    }
}

static void trace_annotation(const dice_ast_node_t *node, void *user_data) {
    trace_visitor_data_t *data = (trace_visitor_data_t *)user_data;
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "key: %s\n", node->data.annotation.key ? node->data.annotation.key : "null");
    
    print_indent(data->output, data->indent_str, data->depth);
    fprintf(data->output, "value: %s\n", node->data.annotation.value ? node->data.annotation.value : "null");
    
    if (node->data.annotation.child) {
        print_indent(data->output, data->indent_str, data->depth);
        fprintf(data->output, "child:\n");
    }
}

// Global storage for trace visitor data (not thread-safe as documented)
static trace_visitor_data_t global_trace_data;

dice_ast_visitor_t dice_create_trace_visitor(FILE *output, const char *indent_str) {
    global_trace_data.output = output ? output : stdout;
    global_trace_data.indent_str = indent_str ? indent_str : "  ";
    global_trace_data.depth = 0;
    
    dice_ast_visitor_t visitor = {0};
    visitor.enter_node = trace_enter_node;
    visitor.exit_node = trace_exit_node;
    visitor.visit_literal = trace_literal;
    visitor.visit_binary_op = trace_binary_op;
    visitor.visit_dice_op = trace_dice_op;
    visitor.visit_function_call = trace_function_call;
    visitor.visit_annotation = trace_annotation;
    visitor.user_data = &global_trace_data;
    
    return visitor;
}