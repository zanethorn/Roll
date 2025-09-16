#ifndef DICE_PARSER_H
#define DICE_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// Forward declarations
typedef struct dice_rng dice_rng_t;
typedef struct dice_expression dice_expression_t;

/**
 * @brief RNG interface - allows swapping out random number generators
 */
struct dice_rng {
    void (*init)(void *state, uint32_t seed);
    int (*roll)(void *state, int sides);
    void *state;
    void (*cleanup)(void *state);
};

/**
 * @brief Expression evaluation result
 */
typedef struct {
    int value;
    int error;
    const char *error_msg;
} dice_eval_result_t;

/**
 * @brief Parse a dice expression string into an AST
 * @param expression_str The dice expression string (e.g., "3d6+2", "2*d20", etc.)
 * @return Parsed expression AST or NULL on error
 */
dice_expression_t* dice_parse_expression(const char *expression_str);

/**
 * @brief Evaluate a parsed expression using the given RNG
 * @param expr The parsed expression
 * @param rng The random number generator to use
 * @return Evaluation result
 */
dice_eval_result_t dice_evaluate_expression(const dice_expression_t *expr, dice_rng_t *rng);

/**
 * @brief Free a parsed expression
 * @param expr The expression to free
 */
void dice_free_expression(dice_expression_t *expr);

/**
 * @brief Create a default RNG (system rand())
 * @param seed Random seed (0 for time-based)
 * @return RNG instance
 */
dice_rng_t* dice_create_default_rng(uint32_t seed);

/**
 * @brief Free an RNG instance
 * @param rng The RNG to free
 */
void dice_free_rng(dice_rng_t *rng);

/**
 * @brief Parse and evaluate an expression in one step (convenience function)
 * @param expression_str The dice expression string
 * @param rng The random number generator to use
 * @return Evaluation result
 */
dice_eval_result_t dice_parse_and_evaluate(const char *expression_str, dice_rng_t *rng);

#ifdef __cplusplus
}
#endif

#endif /* DICE_PARSER_H */