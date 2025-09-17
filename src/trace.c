#include "dice.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
    trace_atomic_roll_selected(ctx, sides, result, false);
}

void trace_atomic_roll_selected(dice_context_t *ctx, int sides, int result, bool selected) {
    dice_trace_entry_t *entry = arena_alloc(ctx, sizeof(dice_trace_entry_t));
    if (!entry) return;
    
    entry->type = TRACE_ATOMIC_ROLL;
    entry->data.atomic_roll.sides = sides;
    entry->data.atomic_roll.result = result;
    entry->data.atomic_roll.selected = selected;
    
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

int dice_format_trace_string(const dice_context_t *ctx, char *buffer, size_t buffer_size) {
    if (!ctx || !buffer || buffer_size == 0) {
        return -1;
    }
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    if (!trace || trace->count == 0) {
        if (buffer_size > 0) {
            buffer[0] = '\0';
        }
        return 0;
    }
    
    size_t pos = 0;
    int written;
    
    // Write header
    written = snprintf(buffer + pos, buffer_size - pos, "Individual dice results:\n");
    if (written < 0 || (size_t)written >= buffer_size - pos) {
        return -1;
    }
    pos += written;
    
    // Iterate through trace entries
    const dice_trace_entry_t *entry = trace->first;
    while (entry && pos < buffer_size - 1) {
        if (entry->type == TRACE_ATOMIC_ROLL) {
            const char *marker = entry->data.atomic_roll.selected ? "*" : "";
            written = snprintf(buffer + pos, buffer_size - pos, 
                             "  d%d -> %d%s\n", 
                             entry->data.atomic_roll.sides, 
                             entry->data.atomic_roll.result,
                             marker);
            if (written < 0 || (size_t)written >= buffer_size - pos) {
                return -1;
            }
            pos += written;
        }
        entry = entry->next;
    }
    
    return (int)pos;
}

int dice_format_trace_stream(const dice_context_t *ctx, FILE *stream) {
    if (!ctx || !stream) {
        return -1;
    }
    
    const dice_trace_t *trace = dice_get_trace(ctx);
    if (!trace || trace->count == 0) {
        return 0;
    }
    
    fprintf(stream, "Individual dice results:\n");
    
    // Iterate through trace entries
    const dice_trace_entry_t *entry = trace->first;
    while (entry) {
        if (entry->type == TRACE_ATOMIC_ROLL) {
            const char *marker = entry->data.atomic_roll.selected ? "*" : "";
            fprintf(stream, "  d%d -> %d%s\n", 
                   entry->data.atomic_roll.sides, 
                   entry->data.atomic_roll.result,
                   marker);
        }
        entry = entry->next;
    }
    
    return 0;
}