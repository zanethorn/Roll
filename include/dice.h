#ifndef DICE_H
#define DICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>

// Forward declarations
typedef struct dice_context dice_context_t;
typedef struct dice_ast_node dice_ast_node_t;
typedef struct dice_rng_vtable dice_rng_vtable_t;
typedef struct dice_policy dice_policy_t;
typedef struct dice_trace dice_trace_t;
typedef struct dice_error_buffer dice_error_buffer_t;
typedef struct dice_ast_visitor dice_ast_visitor_t;

// =============================================================================
// Core Types
// =============================================================================

/**
 * @brief Feature flags for different dice syntax families
 */
typedef enum {
    DICE_FEATURE_BASIC      = 1 << 0,  // Basic NdS notation
    DICE_FEATURE_POOL       = 1 << 1,  // Pool dice (e.g., count successes)
    DICE_FEATURE_EXPLODING  = 1 << 2,  // Exploding dice
    DICE_FEATURE_FATE       = 1 << 3,  // FATE dice (+, -, blank)
    DICE_FEATURE_KEEP_DROP  = 1 << 4,  // Keep highest/lowest N
    DICE_FEATURE_ALL        = 0xFF     // All features enabled
} dice_features_t;

/**
 * @brief AST node types - small tagged union
 */
typedef enum {
    DICE_NODE_LITERAL,       // number literal
    DICE_NODE_BINARY_OP,     // +, -, *, /
    DICE_NODE_DICE_OP,       // NdS dice operation
    DICE_NODE_FUNCTION_CALL, // function(args...)
    DICE_NODE_ANNOTATION     // metadata/annotations
} dice_node_type_t;

/**
 * @brief Binary operators
 */
typedef enum {
    DICE_OP_ADD,
    DICE_OP_SUB,
    DICE_OP_MUL,
    DICE_OP_DIV,
    DICE_OP_GT,     // >
    DICE_OP_LT,     // <
    DICE_OP_GTE,    // >=
    DICE_OP_LTE,    // <=
    DICE_OP_EQ,     // =
    DICE_OP_NEQ     // <>
} dice_binary_op_t;

/**
 * @brief Dice operation types
 */
typedef enum {
    DICE_DICE_BASIC,     // NdS
    DICE_DICE_EXPLODING, // NdS!
    DICE_DICE_POOL,      // NdS pool
    DICE_DICE_FATE,      // NdF FATE dice
    DICE_DICE_FILTER,    // NdS with filter operations (kh/kl/dh/dl/s>N/s<N/etc. unified)
    DICE_DICE_CUSTOM     // Custom dice (NdCUSTOM or Nd{...})
} dice_dice_type_t;

/**
 * @brief Custom die side definition
 */
typedef struct dice_custom_side {
    int64_t value;          // Numeric value for calculations
    const char *label;      // String label (optional, can be NULL)
} dice_custom_side_t;

/**
 * @brief Custom die definition
 */
typedef struct dice_custom_die {
    const char *name;              // Die name (for named dice like "F", "HQ", etc.)
    dice_custom_side_t *sides;     // Array of side definitions
    size_t side_count;             // Number of sides
} dice_custom_die_t;

/**
 * @brief Custom die registry for storing named custom dice
 */
typedef struct dice_custom_die_registry {
    dice_custom_die_t *dice;    // Array of custom dice definitions
    size_t count;               // Number of registered dice
    size_t capacity;            // Maximum capacity
} dice_custom_die_registry_t;

/**
 * @brief Dice selection parameters for unified keep/drop/reroll operations
 */
typedef struct dice_selection {
    int64_t count;         // Number of dice to select (for keep) or drop (for drop)
    bool select_high;      // true for high values, false for low values
    bool is_drop_operation; // true for drop operations, false for keep operations
    const char *original_syntax;  // Original user syntax for trace/output ("kh", "kl", "dh", "dl", "r", etc.)
    // Conditional selection support (for 's' operator)
    bool is_conditional;   // true for conditional selection (s>N, s<N, etc.)
    dice_binary_op_t comparison_op; // Comparison operator for conditional selection
    int64_t comparison_value; // Value to compare against for conditional selection
    // Reroll support (for 'r' operator)
    bool is_reroll;        // true for reroll operations (r, r>N, r<N, etc.)
} dice_selection_t;

/**
 * @brief AST node structure - tagged union
 */
struct dice_ast_node {
    dice_node_type_t type;
    union {
        struct {
            int64_t value;
        } literal;
        
        struct {
            dice_binary_op_t op;
            dice_ast_node_t *left;
            dice_ast_node_t *right;
        } binary_op;
        
        struct {
            dice_dice_type_t dice_type;
            dice_ast_node_t *count;      // number of dice (can be expression)
            dice_ast_node_t *sides;      // sides per die (can be expression, or NULL for custom)
            dice_ast_node_t *modifier;   // keep/drop count, explosion threshold, etc.
            // Dice selection support (unified keep/drop)
            dice_selection_t *selection; // Selection parameters (NULL for non-select operations)
            // Custom dice support
            const char *custom_name;     // Name for named custom dice (e.g., "F", "HQ")
            dice_custom_die_t *custom_die; // Inline custom die definition
        } dice_op;
        
        struct {
            const char *name;
            dice_ast_node_t **args;
            size_t arg_count;
        } function_call;
        
        struct {
            const char *key;
            const char *value;
            dice_ast_node_t *child;
        } annotation;
    } data;
};

/**
 * @brief AST visitor interface for exploring parse trees
 */
struct dice_ast_visitor {
    // Called when entering any node (pre-order)
    void (*enter_node)(const dice_ast_node_t *node, void *user_data);
    
    // Called when exiting any node (post-order)
    void (*exit_node)(const dice_ast_node_t *node, void *user_data);
    
    // Specific node type handlers (optional - can be NULL)
    void (*visit_literal)(const dice_ast_node_t *node, void *user_data);
    void (*visit_binary_op)(const dice_ast_node_t *node, void *user_data);
    void (*visit_dice_op)(const dice_ast_node_t *node, void *user_data);
    void (*visit_function_call)(const dice_ast_node_t *node, void *user_data);
    void (*visit_annotation)(const dice_ast_node_t *node, void *user_data);
    
    // User data pointer passed to all visitor methods
    void *user_data;
};

/**
 * @brief RNG function pointer vtable
 */
struct dice_rng_vtable {
    // Initialize RNG state
    int (*init)(void *state, uint64_t seed);
    
    // Generate random number in range [1, sides]
    int (*roll)(void *state, int sides);
    
    // Generate random number in range [0, max-1]
    uint64_t (*rand)(void *state, uint64_t max);
    
    // Cleanup RNG state
    void (*cleanup)(void *state);
    
    // RNG state data
    void *state;
};

/**
 * @brief Roll trace entry for auditing
 */
typedef struct dice_trace_entry {
    enum {
        TRACE_ATOMIC_ROLL,    // Single die roll
        TRACE_TRANSFORMATION, // Keep/drop/explode operation
        TRACE_EXPRESSION,     // Expression evaluation
        TRACE_FUNCTION_CALL   // Function call
    } type;
    
    union {
        struct {
            int sides;
            int result;
            bool selected;  // true if this die was selected/kept in filtering operations
        } atomic_roll;
        
        struct {
            const char *operation;
            int *input_values;
            int input_count;
            int *output_values;
            int output_count;
        } transformation;
        
        struct {
            const char *expression;
            int64_t result;
        } expression;
        
        struct {
            const char *function_name;
            int64_t result;
        } function_call;
    } data;
    
    struct dice_trace_entry *next;
} dice_trace_entry_t;

/**
 * @brief Structured trace log
 */
struct dice_trace {
    dice_trace_entry_t *first;
    dice_trace_entry_t *last;
    size_t count;
};

/**
 * @brief Policy/configuration for evaluation
 */
struct dice_policy {
    int max_dice_count;      // Maximum dice in single roll
    int max_sides;           // Maximum sides per die
    int max_explosion_depth; // Maximum explosion iterations
    bool allow_negative_dice;// Allow negative die counts
    bool strict_mode;        // Strict parsing/evaluation
};

/**
 * @brief Error buffer for thread-safe error reporting
 */
struct dice_error_buffer {
    char message[1024];
    int code;
    bool has_error;
};

/**
 * @brief Main context handle - contains all state
 */
struct dice_context {
    // Arena allocator for AST nodes
    void *arena;
    size_t arena_size;
    size_t arena_used;
    
    // Error reporting
    dice_error_buffer_t error;
    
    // Feature flags
    dice_features_t features;
    
    // Policy/configuration
    dice_policy_t policy;
    
    // Trace log
    dice_trace_t trace;
    
    // RNG vtable
    dice_rng_vtable_t rng;
    
    // Custom dice registry
    dice_custom_die_registry_t custom_dice;
};

/**
 * @brief Evaluation result
 */
typedef struct {
    int64_t value;
    bool success;
} dice_eval_result_t;

// =============================================================================
// Simple API - Convenience Functions
// =============================================================================

/**
 * @brief Roll a single die with specified number of sides
 * @param sides Number of sides on the die (must be > 0)
 * @return Random value between 1 and sides (inclusive)
 */
int dice_roll(int sides);

/**
 * @brief Roll multiple dice and return the sum
 * @param count Number of dice to roll
 * @param sides Number of sides on each die
 * @return Sum of all dice rolls
 */
int dice_roll_multiple(int count, int sides);

/**
 * @brief Roll multiple dice and store individual results
 * @param count Number of dice to roll
 * @param sides Number of sides on each die
 * @param results Array to store individual dice results (must be at least count elements)
 * @return Sum of all dice rolls
 */
int dice_roll_individual(int count, int sides, int *results);

/**
 * @brief Roll dice using standard RPG notation with full EBNF expression support
 * @param dice_notation String representing dice notation
 * @return Result of the dice roll, or -1 on error
 */
int dice_roll_notation(const char *dice_notation);

/**
 * @brief Instant dice rolling helper - creates context, evaluates expression, and cleans up
 * @param dice_notation String representing dice notation
 * @param seed Random seed (use 0 for time-based randomness)
 * @return Result of the dice roll, or -1 on error
 * @note This is a convenience function that handles all context management internally
 */
int dice_roll_quick(const char *dice_notation, uint32_t seed);

/**
 * @brief Get the version of the dice library
 * @return Version string
 */
const char* dice_version(void);

// =============================================================================
// Advanced API - Context Management
// =============================================================================

/**
 * @brief Create a new dice context with specified arena size
 * @param arena_size Size of arena allocator in bytes
 * @param features Feature flags to enable
 * @return New context handle or NULL on failure
 */
dice_context_t* dice_context_create(size_t arena_size, dice_features_t features);

/**
 * @brief Destroy a dice context and free all resources
 * @param ctx Context to destroy
 */
void dice_context_destroy(dice_context_t *ctx);

/**
 * @brief Reset context arena and clear trace (reuse context)
 * @param ctx Context to reset
 */
void dice_context_reset(dice_context_t *ctx);

/**
 * @brief Set RNG vtable for context
 * @param ctx Context handle
 * @param rng_vtable RNG function pointers and state
 * @return 0 on success, -1 on error
 */
int dice_context_set_rng(dice_context_t *ctx, const dice_rng_vtable_t *rng_vtable);

/**
 * @brief Set policy/configuration for context
 * @param ctx Context handle
 * @param policy Policy configuration
 * @return 0 on success, -1 on error
 */
int dice_context_set_policy(dice_context_t *ctx, const dice_policy_t *policy);

// =============================================================================
// Parsing API
// =============================================================================

/**
 * @brief Parse dice expression string into AST
 * @param ctx Context handle
 * @param expression_str Input expression string
 * @return AST root node or NULL on error
 */
dice_ast_node_t* dice_parse(dice_context_t *ctx, const char *expression_str);

// =============================================================================
// AST Visitor API
// =============================================================================

/**
 * @brief Traverse AST using visitor pattern
 * @param node AST root node to traverse
 * @param visitor Visitor interface with callback functions
 * @note Traverses the tree in depth-first order, calling enter_node, 
 *       specific visitor, then exit_node for each node
 */
void dice_ast_traverse(const dice_ast_node_t *node, const dice_ast_visitor_t *visitor);

/**
 * @brief Create a default tracing visitor that prints AST structure
 * @param output File handle to write output to (e.g., stdout, stderr)
 * @param indent_str String to use for indentation (e.g., "  ", "\t")
 * @return Configured visitor for tracing AST structure
 * @note The returned visitor uses static storage and is not thread-safe
 */
dice_ast_visitor_t dice_create_trace_visitor(FILE *output, const char *indent_str);

// =============================================================================
// Evaluation API
// =============================================================================

/**
 * @brief Evaluate AST node to get roll result
 * @param ctx Context handle (for RNG, tracing, policy)
 * @param node AST node to evaluate
 * @return Evaluation result
 */
dice_eval_result_t dice_evaluate(dice_context_t *ctx, const dice_ast_node_t *node);

/**
 * @brief Parse and evaluate expression in one call
 * @param ctx Context handle
 * @param expression_str Expression to parse and evaluate
 * @return Evaluation result
 */
dice_eval_result_t dice_roll_expression(dice_context_t *ctx, const char *expression_str);

// =============================================================================
// Tracing API
// =============================================================================

/**
 * @brief Get trace log from context
 * @param ctx Context handle
 * @return Trace log (read-only)
 */
const dice_trace_t* dice_get_trace(const dice_context_t *ctx);

/**
 * @brief Clear trace log
 * @param ctx Context handle
 */
void dice_clear_trace(dice_context_t *ctx);

/**
 * @brief Format trace output to a string
 * @param ctx Context handle
 * @param buffer Output buffer
 * @param buffer_size Size of output buffer
 * @return Number of characters written (excluding null terminator), or -1 on error
 */
int dice_format_trace_string(const dice_context_t *ctx, char *buffer, size_t buffer_size);

/**
 * @brief Format trace output to a file stream
 * @param ctx Context handle
 * @param stream Output stream (e.g., stdout, stderr, or file)
 * @return 0 on success, -1 on error
 */
int dice_format_trace_stream(const dice_context_t *ctx, FILE *stream);

// =============================================================================
// Error Handling API
// =============================================================================

/**
 * @brief Check if context has error
 * @param ctx Context handle
 * @return true if error exists
 */
bool dice_has_error(const dice_context_t *ctx);

/**
 * @brief Get error message from context
 * @param ctx Context handle
 * @return Error message string (read-only)
 */
const char* dice_get_error(const dice_context_t *ctx);

/**
 * @brief Clear error in context
 * @param ctx Context handle
 */
void dice_clear_error(dice_context_t *ctx);

// =============================================================================
// Built-in RNG Implementations
// =============================================================================

/**
 * @brief Create default system RNG vtable
 * @param seed Random seed
 * @return RNG vtable
 */
dice_rng_vtable_t dice_create_system_rng(uint64_t seed);

/**
 * @brief Create xoshiro256++ RNG vtable
 * @param seed Random seed
 * @return RNG vtable
 */
dice_rng_vtable_t dice_create_xoshiro_rng(uint64_t seed);

// =============================================================================
// Utility Functions
// =============================================================================

/**
 * @brief Default policy configuration
 * @return Default policy struct
 */
dice_policy_t dice_default_policy(void);

// =============================================================================
// Custom Dice API
// =============================================================================

/**
 * @brief Register a named custom die
 * @param ctx Context handle
 * @param name Die name (e.g., "F" for FATE dice)
 * @param sides Array of side definitions
 * @param side_count Number of sides
 * @return 0 on success, -1 on error
 */
int dice_register_custom_die(dice_context_t *ctx, const char *name, 
                             const dice_custom_side_t *sides, size_t side_count);

/**
 * @brief Create a custom side with value and optional label
 * @param value Numeric value for calculations
 * @param label String label (can be NULL)
 * @return Custom side structure
 */
dice_custom_side_t dice_custom_side(int64_t value, const char *label);

/**
 * @brief Look up a named custom die
 * @param ctx Context handle
 * @param name Die name
 * @return Custom die definition or NULL if not found
 */
const dice_custom_die_t* dice_lookup_custom_die(const dice_context_t *ctx, const char *name);

/**
 * @brief Clear all custom dice from registry
 * @param ctx Context handle
 */
void dice_clear_custom_dice(dice_context_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* DICE_H */