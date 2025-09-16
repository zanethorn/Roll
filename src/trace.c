#include "dice.h"
#include <stdlib.h>
#include <string.h>

// Internal functions for trace
void* arena_alloc(dice_context_t *ctx, size_t size);

// =============================================================================
// Tracing Implementation
// =============================================================================

static void add_trace_entry(dice_context_t *ctx, dice_trace_entry_t *entry) {
    if (!ctx || !entry) return;
    
    entry->next = NULL;
    
    if (!ctx->trace.first) {
        ctx->trace.first = entry;
        ctx->trace.last = entry;
    } else {
        ctx->trace.last->next = entry;
        ctx->trace.last = entry;
    }
    
    ctx->trace.count++;
}

void trace_atomic_roll(dice_context_t *ctx, int sides, int result) {
    dice_trace_entry_t *entry = arena_alloc(ctx, sizeof(dice_trace_entry_t));
    if (!entry) return;
    
    entry->type = TRACE_ATOMIC_ROLL;
    entry->data.atomic_roll.sides = sides;
    entry->data.atomic_roll.result = result;
    
    add_trace_entry(ctx, entry);
}

const dice_trace_t* dice_get_trace(const dice_context_t *ctx) {
    return ctx ? &ctx->trace : NULL;
}

void dice_clear_trace(dice_context_t *ctx) {
    if (ctx) {
        memset(&ctx->trace, 0, sizeof(ctx->trace));
    }
}