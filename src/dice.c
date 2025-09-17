#include "dice.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define DICE_VERSION "2.0.0"

// =============================================================================
// Context Management
// =============================================================================

dice_context_t* dice_context_create(size_t arena_size, dice_features_t features) {
    dice_context_t *ctx = malloc(sizeof(dice_context_t));
    if (!ctx) return NULL;
    
    memset(ctx, 0, sizeof(dice_context_t));
    
    // Allocate arena
    ctx->arena = malloc(arena_size);
    if (!ctx->arena) {
        free(ctx);
        return NULL;
    }
    
    ctx->arena_size = arena_size;
    ctx->arena_used = 0;
    ctx->features = features;
    
    // Set default policy
    ctx->policy = dice_default_policy();
    
    // Set default RNG
    dice_rng_vtable_t rng = dice_create_system_rng(0);
    ctx->rng = rng;
    
    // Initialize custom dice registry
    ctx->custom_dice.dice = NULL;
    ctx->custom_dice.count = 0;
    ctx->custom_dice.capacity = 0;
    
    // Register default FATE dice if FATE feature is enabled
    if (features & DICE_FEATURE_FATE) {
        dice_custom_side_t fate_sides[] = {
            {-1, "-"},
            {0, " "},
            {1, "+"}
        };
        if (dice_register_custom_die(ctx, "F", fate_sides, 3) != 0) {
            // If registration fails, clean up and return NULL
            dice_context_destroy(ctx);
            return NULL;
        }
    }
    
    return ctx;
}

void dice_context_destroy(dice_context_t *ctx) {
    if (!ctx) return;
    
    // Cleanup RNG if needed
    if (ctx->rng.cleanup) {
        ctx->rng.cleanup(ctx->rng.state);
    }
    
    // Cleanup custom dice registry
    dice_clear_custom_dice(ctx);
    free(ctx->custom_dice.dice);
    
    free(ctx->arena);
    free(ctx);
}

void dice_context_reset(dice_context_t *ctx) {
    if (!ctx) return;
    
    // Reset arena
    ctx->arena_used = 0;
    
    // Clear error
    ctx->error.has_error = false;
    ctx->error.code = 0;
    ctx->error.message[0] = '\0';
    
    // Clear trace
    memset(&ctx->trace, 0, sizeof(ctx->trace));
    
    // Clear custom dice registry
    dice_clear_custom_dice(ctx);
}

int dice_context_set_rng(dice_context_t *ctx, const dice_rng_vtable_t *rng_vtable) {
    if (!ctx || !rng_vtable) return -1;
    
    // Cleanup old RNG
    if (ctx->rng.cleanup) {
        ctx->rng.cleanup(ctx->rng.state);
    }
    
    // Copy new RNG
    ctx->rng = *rng_vtable;
    return 0;
}

int dice_context_set_policy(dice_context_t *ctx, const dice_policy_t *policy) {
    if (!ctx || !policy) return -1;
    
    ctx->policy = *policy;
    return 0;
}

// =============================================================================
// Error Handling
// =============================================================================

bool dice_has_error(const dice_context_t *ctx) {
    return ctx ? ctx->error.has_error : true;
}

const char* dice_get_error(const dice_context_t *ctx) {
    return ctx ? ctx->error.message : "Invalid context";
}

void dice_clear_error(dice_context_t *ctx) {
    if (ctx) {
        ctx->error.has_error = false;
        ctx->error.code = 0;
        ctx->error.message[0] = '\0';
    }
}

// =============================================================================
// Utility Functions
// =============================================================================

dice_policy_t dice_default_policy(void) {
    dice_policy_t policy = {
        .max_dice_count = 1000,
        .max_sides = 1000000,
        .max_explosion_depth = 10,
        .allow_negative_dice = false,
        .strict_mode = false
    };
    return policy;
}

const char* dice_version(void) {
    return DICE_VERSION;
}

// =============================================================================
// Simple Wrapper Functions (No Static State)
// =============================================================================

int dice_roll(int sides) {
    return dice_roll_multiple(1, sides);
}

int dice_roll_multiple(int count, int sides) {
    if (count <= 0 || sides <= 0) return -1;
    
    // Create temporary context for this operation
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return -1;
    
    // Use time-based seed for randomness
    dice_rng_vtable_t rng = dice_create_system_rng(0);
    dice_context_set_rng(ctx, &rng);
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, sides);
        if (roll < 0) {
            dice_context_destroy(ctx);
            return -1;
        }
        sum += roll;
    }
    
    dice_context_destroy(ctx);
    return sum;
}

int dice_roll_individual(int count, int sides, int *results) {
    if (count <= 0 || sides <= 0 || !results) return -1;
    
    // Create temporary context for this operation
    dice_context_t *ctx = dice_context_create(1024, DICE_FEATURE_BASIC);
    if (!ctx) return -1;
    
    // Use time-based seed for randomness
    dice_rng_vtable_t rng = dice_create_system_rng(0);
    dice_context_set_rng(ctx, &rng);
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, sides);
        if (roll < 0) {
            dice_context_destroy(ctx);
            return -1;
        }
        results[i] = roll;
        sum += roll;
    }
    
    dice_context_destroy(ctx);
    return sum;
}

int dice_roll_notation(const char *dice_notation) {
    if (!dice_notation) return -1;
    
    // Create temporary context for this operation
    dice_context_t *ctx = dice_context_create(4096, DICE_FEATURE_ALL);
    if (!ctx) return -1;
    
    // Use time-based seed for randomness
    dice_rng_vtable_t rng = dice_create_system_rng(0);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result = dice_roll_expression(ctx, dice_notation);
    
    int ret_val = -1;
    if (result.success && !dice_has_error(ctx)) {
        ret_val = (int)result.value;
    }
    
    dice_context_destroy(ctx);
    return ret_val;
}

int dice_roll_quick(const char *dice_notation, uint32_t seed) {
    if (!dice_notation) return -1;
    
    // Create temporary context for this operation
    dice_context_t *ctx = dice_context_create(4096, DICE_FEATURE_ALL);
    if (!ctx) return -1;
    
    // Use provided seed or time-based for randomness
    dice_rng_vtable_t rng = dice_create_system_rng(seed);
    dice_context_set_rng(ctx, &rng);
    
    dice_eval_result_t result = dice_roll_expression(ctx, dice_notation);
    
    int ret_val = -1;
    if (result.success && !dice_has_error(ctx)) {
        ret_val = (int)result.value;
    }
    
    dice_context_destroy(ctx);
    return ret_val;
}