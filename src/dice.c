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

// =============================================================================
// Custom Dice Implementation
// =============================================================================

dice_custom_side_t dice_custom_side(int64_t value, const char *label) {
    dice_custom_side_t side;
    side.value = value;
    side.label = label ? strdup(label) : NULL;
    return side;
}

int dice_register_custom_die(dice_context_t *ctx, const char *name, 
                             const dice_custom_side_t *sides, size_t side_count) {
    if (!ctx || !name || !sides || side_count == 0) return -1;
    
    // Check if we need to expand the registry
    if (ctx->custom_dice.count >= ctx->custom_dice.capacity) {
        size_t new_capacity = ctx->custom_dice.capacity == 0 ? 4 : ctx->custom_dice.capacity * 2;
        dice_custom_die_t *new_dice = realloc(ctx->custom_dice.dice, 
                                              new_capacity * sizeof(dice_custom_die_t));
        if (!new_dice) {
            snprintf(ctx->error.message, sizeof(ctx->error.message),
                    "Failed to allocate memory for custom dice registry");
            ctx->error.has_error = true;
            return -1;
        }
        ctx->custom_dice.dice = new_dice;
        ctx->custom_dice.capacity = new_capacity;
    }
    
    // Create the custom die
    dice_custom_die_t *die = &ctx->custom_dice.dice[ctx->custom_dice.count];
    die->name = strdup(name);
    die->side_count = side_count;
    die->sides = malloc(side_count * sizeof(dice_custom_side_t));
    if (!die->sides) {
        free((void*)die->name);
        snprintf(ctx->error.message, sizeof(ctx->error.message),
                "Failed to allocate memory for custom die sides");
        ctx->error.has_error = true;
        return -1;
    }
    
    // Copy sides
    for (size_t i = 0; i < side_count; i++) {
        die->sides[i].value = sides[i].value;
        die->sides[i].label = sides[i].label ? strdup(sides[i].label) : NULL;
    }
    
    ctx->custom_dice.count++;
    
    return 0;
}

const dice_custom_die_t* dice_lookup_custom_die(const dice_context_t *ctx, const char *name) {
    if (!ctx || !name) return NULL;
    
    for (size_t i = 0; i < ctx->custom_dice.count; i++) {
        if (strcmp(ctx->custom_dice.dice[i].name, name) == 0) {
            return &ctx->custom_dice.dice[i];
        }
    }
    
    return NULL;
}

void dice_clear_custom_dice(dice_context_t *ctx) {
    if (!ctx) return;
    
    for (size_t i = 0; i < ctx->custom_dice.count; i++) {
        dice_custom_die_t *die = &ctx->custom_dice.dice[i];
        free((void*)die->name);
        
        // Free labels from sides
        for (size_t j = 0; j < die->side_count; j++) {
            free((void*)die->sides[j].label);
        }
        free(die->sides);
    }
    
    ctx->custom_dice.count = 0;
}