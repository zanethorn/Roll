#include "dice.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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