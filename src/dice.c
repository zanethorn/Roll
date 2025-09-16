#include "dice.h"
#include <stdlib.h>
#include <string.h>

#define DICE_VERSION "2.0.0"

// Global context for backward compatibility (not thread-safe, but maintains API compatibility)
static dice_context_t *legacy_context = NULL;

static dice_context_t* get_legacy_context(void) {
    if (!legacy_context) {
        // Create a default context with 1MB arena and all features
        legacy_context = dice_context_create(1024 * 1024, DICE_FEATURE_ALL);
    }
    return legacy_context;
}

void dice_init(uint32_t seed) {
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return;
    
    // Set system RNG with specified seed
    dice_rng_vtable_t rng = dice_create_system_rng(seed ? seed : 0);
    dice_context_set_rng(ctx, &rng);
}

int dice_roll(int sides) {
    if (sides <= 0) return -1;
    
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return -1;
    
    return ctx->rng.roll(ctx->rng.state, sides);
}

int dice_roll_multiple(int count, int sides) {
    if (count <= 0 || sides <= 0) return -1;
    
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return -1;
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, sides);
        if (roll < 0) return -1;
        sum += roll;
    }
    return sum;
}

int dice_roll_individual(int count, int sides, int *results) {
    if (count <= 0 || sides <= 0 || !results) return -1;
    
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return -1;
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        int roll = ctx->rng.roll(ctx->rng.state, sides);
        if (roll < 0) return -1;
        results[i] = roll;
        sum += roll;
    }
    return sum;
}

int dice_roll_notation(const char *dice_notation) {
    if (!dice_notation) return -1;
    
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return -1;
    
    // Clear any previous errors
    dice_clear_error(ctx);
    
    // Use the new architecture
    dice_eval_result_t result = dice_roll_expression(ctx, dice_notation);
    
    if (!result.success || dice_has_error(ctx)) {
        return -1;
    }
    
    return (int)result.value;
}

const char* dice_version(void) {
    return DICE_VERSION;
}

void dice_set_rng(const dice_rng_vtable_t *rng_vtable) {
    if (!rng_vtable) return;
    
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return;
    
    dice_context_set_rng(ctx, rng_vtable);
}

const dice_rng_vtable_t* dice_get_rng(void) {
    dice_context_t *ctx = get_legacy_context();
    if (!ctx) return NULL;
    
    return &ctx->rng;
}

void dice_cleanup(void) {
    if (legacy_context) {
        dice_context_destroy(legacy_context);
        legacy_context = NULL;
    }
}