#include "dice.h"
#include "dice_parser.h"
#include <stdlib.h>
#include <time.h>

#define DICE_VERSION "1.0.0"

static dice_rng_t *global_rng = NULL;

void dice_init(uint32_t seed) {
    if (global_rng) {
        dice_free_rng(global_rng);
    }
    global_rng = dice_create_default_rng(seed);
}

static dice_rng_t* ensure_rng_initialized(void) {
    if (!global_rng) {
        dice_init(0); // Initialize with time-based seed
    }
    return global_rng;
}

int dice_roll(int sides) {
    if (sides <= 0) {
        return -1;
    }
    
    dice_rng_t *rng = ensure_rng_initialized();
    if (!rng) {
        return -1;
    }
    
    return rng->roll(rng->state, sides);
}

int dice_roll_multiple(int count, int sides) {
    if (count <= 0 || sides <= 0) {
        return -1;
    }
    
    dice_rng_t *rng = ensure_rng_initialized();
    if (!rng) {
        return -1;
    }
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        int roll_result = rng->roll(rng->state, sides);
        if (roll_result < 0) {
            return -1;
        }
        sum += roll_result;
    }
    return sum;
}

int dice_roll_individual(int count, int sides, int *results) {
    if (count <= 0 || sides <= 0 || results == NULL) {
        return -1;
    }
    
    dice_rng_t *rng = ensure_rng_initialized();
    if (!rng) {
        return -1;
    }
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        int roll_result = rng->roll(rng->state, sides);
        if (roll_result < 0) {
            return -1;
        }
        results[i] = roll_result;
        sum += roll_result;
    }
    return sum;
}

int dice_roll_notation(const char *dice_notation) {
    if (dice_notation == NULL) {
        return -1;
    }
    
    dice_rng_t *rng = ensure_rng_initialized();
    if (!rng) {
        return -1;
    }
    
    // Use the new EBNF parser
    dice_eval_result_t result = dice_parse_and_evaluate(dice_notation, rng);
    
    if (result.error) {
        return -1;
    }
    
    return result.value;
}

const char* dice_version(void) {
    return DICE_VERSION;
}

void dice_set_rng(dice_rng_t *rng) {
    if (global_rng) {
        dice_free_rng(global_rng);
    }
    global_rng = rng;
}

dice_rng_t* dice_get_rng(void) {
    return ensure_rng_initialized();
}