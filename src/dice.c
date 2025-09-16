#include "dice.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

#define DICE_VERSION "1.0.0"

static int dice_initialized = 0;

void dice_init(uint32_t seed) {
    if (seed == 0) {
        seed = (uint32_t)time(NULL);
    }
    srand(seed);
    dice_initialized = 1;
}

static void ensure_initialized(void) {
    if (!dice_initialized) {
        dice_init(0);
    }
}

int dice_roll(int sides) {
    if (sides <= 0) {
        return -1;
    }
    
    ensure_initialized();
    return (rand() % sides) + 1;
}

int dice_roll_multiple(int count, int sides) {
    if (count <= 0 || sides <= 0) {
        return -1;
    }
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        sum += dice_roll(sides);
    }
    return sum;
}

int dice_roll_individual(int count, int sides, int *results) {
    if (count <= 0 || sides <= 0 || results == NULL) {
        return -1;
    }
    
    int sum = 0;
    for (int i = 0; i < count; i++) {
        results[i] = dice_roll(sides);
        sum += results[i];
    }
    return sum;
}

int dice_roll_notation(const char *dice_notation) {
    if (dice_notation == NULL) {
        return -1;
    }
    
    // Parse notation like "3d6" or "1d20+5" or "2d8-1"
    int count = 0, sides = 0, modifier = 0;
    char *endptr;
    const char *ptr = dice_notation;
    
    // Skip whitespace
    while (isspace(*ptr)) ptr++;
    
    // Parse count
    count = (int)strtol(ptr, &endptr, 10);
    if (ptr == endptr || count <= 0) {
        return -1;
    }
    ptr = endptr;
    
    // Expect 'd' or 'D'
    if (*ptr != 'd' && *ptr != 'D') {
        return -1;
    }
    ptr++;
    
    // Parse sides
    sides = (int)strtol(ptr, &endptr, 10);
    if (ptr == endptr || sides <= 0) {
        return -1;
    }
    ptr = endptr;
    
    // Parse optional modifier
    while (isspace(*ptr)) ptr++;
    if (*ptr == '+' || *ptr == '-') {
        char op = *ptr++;
        modifier = (int)strtol(ptr, &endptr, 10);
        if (ptr != endptr) {
            if (op == '-') {
                modifier = -modifier;
            }
        }
    }
    
    // Roll the dice
    int result = dice_roll_multiple(count, sides);
    if (result < 0) {
        return -1;
    }
    
    return result + modifier;
}

const char* dice_version(void) {
    return DICE_VERSION;
}