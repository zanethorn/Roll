#ifndef INTERNAL_H
#define INTERNAL_H

#include "dice.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// Internal Functions - Not exposed to external clients
// =============================================================================

/**
 * @brief Arena allocator for internal memory management
 * @param ctx Context handle containing the arena
 * @param size Number of bytes to allocate
 * @return Pointer to allocated memory or NULL on failure
 * @note Memory is aligned to 8-byte boundary and zero-initialized
 */
void* arena_alloc(dice_context_t *ctx, size_t size);

/**
 * @brief Add atomic roll entry to trace log
 * @param ctx Context handle for tracing
 * @param sides Number of sides on the die
 * @param result The actual roll result
 */
void trace_atomic_roll(dice_context_t *ctx, int sides, int result);

#ifdef __cplusplus
}
#endif

#endif /* INTERNAL_H */