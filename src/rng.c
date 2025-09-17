#include "dice.h"
#include "internal.h"
#include <stdlib.h>
#include <time.h>

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