#include "dice.h"
#include "internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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