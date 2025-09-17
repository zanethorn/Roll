# API Reference

Complete documentation for the Roll dice library API.

## Overview

The Roll library provides two API levels:
- **Basic API**: Simple global state functions for basic usage
- **Advanced API**: Context-based functions for thread safety and advanced features

## Basic API (Simple Global State)

These functions use global state and are perfect for simple use cases:

### Initialization and Cleanup

```c
void dice_init(uint32_t seed);
void dice_cleanup(void);
const char* dice_version(void);
```

- **`dice_init(seed)`** - Initialize with optional seed (0 = time-based)
- **`dice_cleanup()`** - Cleanup global resources
- **`dice_version()`** - Get library version string

### Basic Dice Rolling

```c
int dice_roll(int sides);
int dice_roll_multiple(int count, int sides);
int dice_roll_individual(int count, int sides, int* results);
```

- **`dice_roll(sides)`** - Roll single die (1 to sides), returns result or -1 on error
- **`dice_roll_multiple(count, sides)`** - Roll multiple dice, return sum or -1 on error
- **`dice_roll_individual(count, sides, results)`** - Roll multiple dice, store individual results, return sum or -1 on error

### Notation Parsing

```c
int dice_roll_notation(const char* notation);
```

- **`dice_roll_notation(notation)`** - Parse and roll RPG notation, return result or -1 on error

## Advanced API (Context-Based)

For thread-safety, custom configuration, and advanced features.

### Context Management

```c
typedef struct dice_context dice_context_t;

dice_context_t* dice_context_create(size_t arena_size, dice_features_t features);
void dice_context_destroy(dice_context_t* ctx);
void dice_context_reset(dice_context_t* ctx);
```

- **`dice_context_create(arena_size, features)`** - Create new context with memory arena and feature flags
- **`dice_context_destroy(ctx)`** - Cleanup context and all associated resources
- **`dice_context_reset(ctx)`** - Reset context for reuse (clears state, keeps configuration)

### Parsing and Evaluation

```c
typedef struct dice_ast_node dice_ast_node_t;

dice_ast_node_t* dice_parse(dice_context_t* ctx, const char* expression);
int dice_evaluate(dice_context_t* ctx, const dice_ast_node_t* node);
int dice_roll_expression(dice_context_t* ctx, const char* expression);
```

- **`dice_parse(ctx, expression)`** - Parse expression to AST, returns node or NULL on error
- **`dice_evaluate(ctx, node)`** - Evaluate AST node, returns result or -1 on error  
- **`dice_roll_expression(ctx, expression)`** - Parse and evaluate in one call

### Configuration

```c
typedef struct dice_rng_vtable dice_rng_vtable_t;
typedef struct dice_policy dice_policy_t;

void dice_context_set_rng(dice_context_t* ctx, const dice_rng_vtable_t* rng);
void dice_context_set_policy(dice_context_t* ctx, const dice_policy_t* policy);
dice_rng_vtable_t* dice_get_rng(void);
```

- **`dice_context_set_rng(ctx, rng)`** - Set custom random number generator
- **`dice_context_set_policy(ctx, policy)`** - Set evaluation policies and limits
- **`dice_get_rng()`** - Get default RNG vtable

### Tracing and Debugging

```c
typedef struct dice_trace dice_trace_t;

const dice_trace_t* dice_get_trace(dice_context_t* ctx);
void dice_clear_trace(dice_context_t* ctx);
bool dice_has_error(dice_context_t* ctx);
const char* dice_get_error(dice_context_t* ctx);
```

- **`dice_get_trace(ctx)`** - Get detailed roll trace
- **`dice_clear_trace(ctx)`** - Clear trace log
- **`dice_has_error(ctx)`** - Check if context has error state
- **`dice_get_error(ctx)`** - Get error message string

## Feature Flags

Control which dice notation features are enabled:

```c
typedef enum {
    DICE_FEATURE_BASIC      = 1 << 0,  // Basic NdS notation
    DICE_FEATURE_POOL       = 1 << 1,  // Pool dice (count successes)
    DICE_FEATURE_EXPLODING  = 1 << 2,  // Exploding dice
    DICE_FEATURE_FATE       = 1 << 3,  // FATE dice (+, -, blank)
    DICE_FEATURE_KEEP_DROP  = 1 << 4,  // Keep highest/lowest N
    DICE_FEATURE_ALL        = 0xFF     // All features enabled
} dice_features_t;
```

## Supported Notation

### Basic Notation ✅

Currently implemented and fully supported:

- **`1d6`, `3d6`, `1d20`** - Standard dice notation
- **`1d20+5`, `2d8-1`** - Dice with modifiers  
- **`d6`** - Implicit single die (same as `1d6`)
- **`2*3`, `10/2`, `(2+3)*4`** - Mathematical expressions
- **`2d6+1d4`** - Mixed dice expressions
- **Case insensitive**: `1D6` and `1d6` are equivalent

### Advanced Notation 🔄 

Planned features (architecture supports, implementation in progress):

- **`1d6!`** - Exploding dice (reroll on maximum)
- **`4d6kh3`** - Keep highest 3 of 4d6
- **`4d6dl1`** - Drop lowest 1 of 4d6  
- **`6d6>4`** - Count successes (≥ target number)
- **`4df`** - FATE dice (+1, -1, 0)

## Usage Examples

### Basic Usage

```c
#include "dice.h"

// Simple usage with global state
dice_init(12345);  // Seed for reproducible results

int result = dice_roll(6);  // Roll 1d6
int sum = dice_roll_multiple(3, 6);  // Roll 3d6, get sum
int total = dice_roll_notation("3d6+2");  // RPG notation

dice_cleanup();  // Clean up resources
```

### Advanced Usage with Context

```c
#include "dice.h"

// Create context for thread-safe usage
dice_context_t* ctx = dice_context_create(8192, DICE_FEATURE_ALL);

// Parse and evaluate separately
dice_ast_node_t* ast = dice_parse(ctx, "2d6+3");
if (ast != NULL) {
    int result = dice_evaluate(ctx, ast);
    printf("Result: %d\n", result);
} else {
    printf("Parse error: %s\n", dice_get_error(ctx));
}

// Get detailed trace
const dice_trace_t* trace = dice_get_trace(ctx);
// ... process trace ...

dice_context_destroy(ctx);
```

### Individual Die Results

```c
int results[4];
int total = dice_roll_individual(4, 6, results);

printf("Total: %d\n", total);
printf("Individual: ");
for (int i = 0; i < 4; i++) {
    printf("%d ", results[i]);
}
printf("\n");
```

### Custom Random Number Generator

```c
// Define custom RNG functions
uint32_t my_rand32(void* state) {
    // Your RNG implementation
    return rand();
}

uint64_t my_rand64(void* state) {
    return ((uint64_t)rand() << 32) | rand();
}

// Create RNG vtable
dice_rng_vtable_t my_rng = {
    .state = NULL,
    .rand32 = my_rand32,
    .rand64 = my_rand64
};

// Use with context
dice_context_t* ctx = dice_context_create(8192, DICE_FEATURE_BASIC);
dice_context_set_rng(ctx, &my_rng);

int result = dice_roll_expression(ctx, "3d6");
```

## Return Values and Error Handling

### Return Value Conventions

- **Positive values**: Successful dice roll results
- **Zero**: Valid result (possible from expressions like "1d6-6")
- **Negative values**: Error conditions
  - `-1`: General error (invalid parameters, parse failure, etc.)
  - Other negative values: Reserved for future specific error codes

### Error Checking

```c
// Basic API - check return value
int result = dice_roll_notation("3d6+2");
if (result < 0) {
    printf("Error rolling dice\n");
} else {
    printf("Result: %d\n", result);
}

// Advanced API - use context error state
dice_context_t* ctx = dice_context_create(8192, DICE_FEATURE_BASIC);
int result = dice_roll_expression(ctx, "invalid notation");
if (dice_has_error(ctx)) {
    printf("Error: %s\n", dice_get_error(ctx));
}
```

## Thread Safety

- **Basic API**: Not thread-safe (uses global state)
- **Advanced API**: Thread-safe when using separate contexts
- **Shared Contexts**: Not safe for concurrent access (use one context per thread)

## Memory Management

- **Basic API**: Automatically managed, call `dice_cleanup()` when done
- **Advanced API**: Context-based with arena allocation
  - Create contexts with `dice_context_create()`
  - Destroy contexts with `dice_context_destroy()`
  - Reset for reuse with `dice_context_reset()`

## Build Configuration

When building the library, use these CMake options:

```bash
# Static library (default)
cmake ..

# Shared library  
cmake -DBUILD_SHARED_LIBS=ON ..

# Disable console app
cmake -DBUILD_CONSOLE_APP=OFF ..

# Disable tests
cmake -DBUILD_TESTS=OFF ..
```