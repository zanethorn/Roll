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
// Legacy API Wrapper Functions (Stateless)
// =============================================================================

// Global state for legacy API compatibility (thread-local if available)
#ifdef _Thread_local
static _Thread_local uint32_t legacy_seed = 0;
static _Thread_local dice_rng_vtable_t *legacy_rng = NULL;
static _Thread_local int legacy_rng_counter = 0;
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L && !defined(__STDC_NO_THREADS__)
static _Thread_local uint32_t legacy_seed = 0;
static _Thread_local dice_rng_vtable_t *legacy_rng = NULL;
static _Thread_local int legacy_rng_counter = 0;
#else
static uint32_t legacy_seed = 0;
static dice_rng_vtable_t *legacy_rng = NULL;
static int legacy_rng_counter = 0;
#endif

void dice_init(uint32_t seed) {
    legacy_seed = seed;
    legacy_rng_counter = 0;
    
    // Clean up any existing legacy RNG
    if (legacy_rng) {
        if (legacy_rng->cleanup) {
            legacy_rng->cleanup(legacy_rng->state);
        }
        free(legacy_rng);
        legacy_rng = NULL;
    }
}

static dice_rng_vtable_t* get_legacy_rng(void) {
    if (!legacy_rng) {
        legacy_rng = malloc(sizeof(dice_rng_vtable_t));
        if (legacy_rng) {
            *legacy_rng = dice_create_system_rng(legacy_seed);
        }
    }
    return legacy_rng;
}

int dice_roll(int sides) {
    if (sides <= 0) return -1;
    
    dice_rng_vtable_t *rng = get_legacy_rng();
    if (!rng) return -1;
    
    // Use counter to ensure different results for same seed
    legacy_rng_counter++;
    return rng->roll(rng->state, sides);
}

int dice_roll_multiple(int count, int sides) {
    if (count <= 0 || sides <= 0) return -1;
    
    dice_rng_vtable_t *rng = get_legacy_rng();
    if (!rng) return -1;
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        legacy_rng_counter++;
        int roll = rng->roll(rng->state, sides);
        if (roll < 0) return -1;
        sum += roll;
    }
    
    return sum;
}

int dice_roll_individual(int count, int sides, int *results) {
    if (count <= 0 || sides <= 0 || !results) return -1;
    
    dice_rng_vtable_t *rng = get_legacy_rng();
    if (!rng) return -1;
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        legacy_rng_counter++;
        int roll = rng->roll(rng->state, sides);
        if (roll < 0) return -1;
        results[i] = roll;
        sum += roll;
    }
    
    return sum;
}

int dice_roll_notation(const char *dice_notation) {
    if (!dice_notation) return -1;
    
    dice_context_t *ctx = dice_context_create(4096, DICE_FEATURE_ALL);
    if (!ctx) return -1;
    
    // Create a separate RNG for this context to avoid sharing state
    dice_rng_vtable_t rng = dice_create_system_rng(legacy_seed + legacy_rng_counter);
    dice_context_set_rng(ctx, &rng);
    
    legacy_rng_counter++;
    
    dice_eval_result_t result = dice_roll_expression(ctx, dice_notation);
    
    int ret_val = -1;
    if (result.success && !dice_has_error(ctx)) {
        ret_val = (int)result.value;
    }
    
    dice_context_destroy(ctx);
    return ret_val;
}

void dice_set_rng(const dice_rng_vtable_t *rng_vtable) {
    if (!rng_vtable) return;
    
    // Clean up existing legacy RNG
    if (legacy_rng) {
        if (legacy_rng->cleanup) {
            legacy_rng->cleanup(legacy_rng->state);
        }
        free(legacy_rng);
    }
    
    // Create a deep copy of the RNG - we need to avoid sharing state
    legacy_rng = malloc(sizeof(dice_rng_vtable_t));
    if (legacy_rng) {
        // Copy the function pointers
        legacy_rng->init = rng_vtable->init;
        legacy_rng->roll = rng_vtable->roll;
        legacy_rng->rand = rng_vtable->rand;
        legacy_rng->cleanup = rng_vtable->cleanup;
        
        // Create our own copy of the state by creating a new RNG with same seed
        // This is a bit of a hack but avoids the double-free issue
        if (rng_vtable->state) {
            *legacy_rng = dice_create_system_rng(legacy_seed);
        } else {
            legacy_rng->state = NULL;
        }
    }
}

const dice_rng_vtable_t* dice_get_rng(void) {
    return get_legacy_rng();
}

void dice_cleanup(void) {
    if (legacy_rng) {
        if (legacy_rng->cleanup) {
            legacy_rng->cleanup(legacy_rng->state);
        }
        free(legacy_rng);
        legacy_rng = NULL;
    }
    legacy_rng_counter = 0;
}