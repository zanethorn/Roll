#include "dice.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define DICE_VERSION "2.0.0"

// =============================================================================
// Arena Allocator Implementation
// =============================================================================

void* arena_alloc(dice_context_t *ctx, size_t size) {
    // Align to 8-byte boundary
    size = (size + 7) & ~7;
    
    if (ctx->arena_used + size > ctx->arena_size) {
        snprintf(ctx->error.message, sizeof(ctx->error.message), 
                "Arena allocator out of memory: requested %zu, available %zu", 
                size, ctx->arena_size - ctx->arena_used);
        ctx->error.code = -1;
        ctx->error.has_error = true;
        return NULL;
    }
    
    void *ptr = (char*)ctx->arena + ctx->arena_used;
    ctx->arena_used += size;
    memset(ptr, 0, size);
    return ptr;
}

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
    
    return ctx;
}

void dice_context_destroy(dice_context_t *ctx) {
    if (!ctx) return;
    
    // Cleanup RNG if needed
    if (ctx->rng.cleanup) {
        ctx->rng.cleanup(ctx->rng.state);
    }
    
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
// RNG Implementations
// =============================================================================

// System RNG state
typedef struct {
    uint64_t seed;
} system_rng_state_t;

static int system_rng_init(void *state, uint64_t seed) {
    system_rng_state_t *s = (system_rng_state_t*)state;
    s->seed = seed ? seed : (uint64_t)time(NULL);
    srand((unsigned int)s->seed);
    return 0;
}

static int system_rng_roll(void *state, int sides) {
    (void)state; // unused
    if (sides <= 0) return -1;
    return (rand() % sides) + 1;
}

static uint64_t system_rng_rand(void *state, uint64_t max) {
    (void)state; // unused
    if (max == 0) return 0;
    return rand() % max;
}

static void system_rng_cleanup(void *state) {
    free(state);
}

dice_rng_vtable_t dice_create_system_rng(uint64_t seed) {
    system_rng_state_t *state = malloc(sizeof(system_rng_state_t));
    
    dice_rng_vtable_t rng = {
        .init = system_rng_init,
        .roll = system_rng_roll,
        .rand = system_rng_rand,
        .cleanup = system_rng_cleanup,
        .state = state
    };
    
    rng.init(state, seed);
    return rng;
}

// Placeholder for xoshiro - just use system RNG for now
dice_rng_vtable_t dice_create_xoshiro_rng(uint64_t seed) {
    return dice_create_system_rng(seed);
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

// =============================================================================
// Legacy API - Global context for backward compatibility
// =============================================================================

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