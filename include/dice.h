#ifndef DICE_H
#define DICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "dice_core.h"

/**
 * @brief Initialize the dice library with a seed for random number generation
 * @param seed Random seed value (use 0 for time-based seed)
 */
void dice_init(uint32_t seed);

/**
 * @brief Roll a single die with specified number of sides
 * @param sides Number of sides on the die (must be > 0)
 * @return Random value between 1 and sides (inclusive)
 */
int dice_roll(int sides);

/**
 * @brief Roll multiple dice and return the sum
 * @param count Number of dice to roll
 * @param sides Number of sides on each die
 * @return Sum of all dice rolls
 */
int dice_roll_multiple(int count, int sides);

/**
 * @brief Roll multiple dice and store individual results
 * @param count Number of dice to roll
 * @param sides Number of sides on each die
 * @param results Array to store individual dice results (must be at least count elements)
 * @return Sum of all dice rolls
 */
int dice_roll_individual(int count, int sides, int *results);

/**
 * @brief Roll dice using standard RPG notation with full EBNF expression support
 * @param dice_notation String representing dice notation
 * @return Result of the dice roll, or -1 on error
 */
int dice_roll_notation(const char *dice_notation);

/**
 * @brief Get the version of the dice library
 * @return Version string
 */
const char* dice_version(void);

/**
 * @brief Set a custom RNG for the library (optional)
 * @param rng_vtable Custom RNG vtable
 */
void dice_set_rng(const dice_rng_vtable_t *rng_vtable);

/**
 * @brief Get the current RNG vtable
 * @return Current RNG vtable (read-only)
 */
const dice_rng_vtable_t* dice_get_rng(void);

/**
 * @brief Cleanup dice library resources
 */
void dice_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* DICE_H */