# API Reference

Complete documentation for the Roll dice library API.

## Overview

The Roll library provides two API levels:
- **Simple API**: Convenient wrapper functions for quick dice rolling
- **Context-Based API**: Full control over memory, RNG, and advanced features

## Simple API (Convenient Wrappers)

These functions provide easy-to-use dice rolling with automatic context management:

### Basic Dice Rolling

```c
int dice_roll(int sides);
int dice_roll_multiple(int count, int sides);
int dice_roll_individual(int count, int sides, int* results);
```

### Notation Parsing

```c
int dice_roll_notation(const char* notation);
int dice_roll_quick(const char* notation, uint32_t seed);
```

- **`dice_roll_notation(notation)`** - Parse and roll RPG notation with time-based randomness
- **`dice_roll_quick(notation, seed)`** - Parse and roll with specific seed for reproducible results

### Utility

```c
const char* dice_version(void);
```

## Context-Based API (Advanced)

The context-based API provides thread safety, better performance, and advanced features by managing state explicitly through context objects.

### Context Management

```c
dice_context_t* dice_context_create(size_t arena_size, dice_features_t features);
void dice_context_destroy(dice_context_t* ctx);
void dice_context_reset(dice_context_t* ctx);
```

- **`dice_context_create(arena_size, features)`** - Create new context with specified memory arena and feature flags
- **`dice_context_destroy(ctx)`** - Destroy context and free all resources
- **`dice_context_reset(ctx)`** - Reset context arena and clear state for reuse

### Configuration

```c
int dice_context_set_rng(dice_context_t* ctx, const dice_rng_vtable_t* rng_vtable);
int dice_context_set_policy(dice_context_t* ctx, const dice_policy_t* policy);
dice_policy_t dice_default_policy(void);
```

### Rolling Dice

```c
dice_eval_result_t dice_roll_expression(dice_context_t* ctx, const char* expression_str);
dice_ast_node_t* dice_parse(dice_context_t* ctx, const char* expression_str);
dice_eval_result_t dice_evaluate(dice_context_t* ctx, const dice_ast_node_t* node);
```

### Error Handling

```c
bool dice_has_error(const dice_context_t* ctx);
const char* dice_get_error(const dice_context_t* ctx);
void dice_clear_error(dice_context_t* ctx);
```

### RNG Support

```c
dice_rng_vtable_t dice_create_system_rng(uint64_t seed);
dice_rng_vtable_t dice_create_xoshiro_rng(uint64_t seed);
```

### Custom Dice

```c
dice_custom_side_t dice_custom_side(int64_t value, const char* label);
int dice_register_custom_die(dice_context_t* ctx, const char* name, 
                             const dice_custom_side_t* sides, size_t side_count);
const dice_custom_die_t* dice_lookup_custom_die(const dice_context_t* ctx, const char* name);
void dice_clear_custom_dice(dice_context_t* ctx);
```