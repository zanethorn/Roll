#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dice.h"

void print_usage(const char *program_name) {
    printf("Usage: %s [options] <dice_notation>\n", program_name);
    printf("  dice_notation: Standard RPG notation (e.g., '3d6', '1d20+5', '2d8-1')\n");
    printf("                 or custom dice notation (e.g., '1d{-1,0,1}', '1dF')\n");
    printf("  Options:\n");
    printf("    -h, --help        Show this help message\n");
    printf("    -v, --version     Show version information\n");
    printf("    -s, --seed N      Set random seed to N\n");
    printf("    -c, --count N     Roll N times\n");
    printf("    -i, --individual  Show individual dice results\n");
    printf("    --die NAME=DEF    Define a named custom die\n");
    printf("\n");
    printf("  Custom Die Examples:\n");
    printf("    %s '1d{-1,0,1}'                    # Inline FATE die\n", program_name);
    printf("    %s --die F={-1,0,1} '4dF'         # Named FATE dice\n", program_name);
    printf("    %s --die HQ='{0:\"Skull\",1:\"Shield\"}' '1dHQ'  # Labeled dice\n", program_name);
    printf("    %s '1d{\"Earth\",\"Wind\",\"Fire\"}'      # String-only dice\n", program_name);
    printf("\n");
    printf("  Standard Examples:\n");
    printf("    %s 3d6        # Roll 3 six-sided dice\n", program_name);
    printf("    %s 1d20+5     # Roll 1 twenty-sided die with +5 modifier\n", program_name);
    printf("    %s -c 5 2d8   # Roll 2 eight-sided dice 5 times\n", program_name);
    printf("    %s -i 4d6     # Roll 4 six-sided dice, show individual results\n", program_name);
}

int main(int argc, char *argv[]) {
    uint32_t seed = 0;
    int count = 1;
    int show_individual = 0;
    char *dice_notation = NULL;
    
    // Create dice context for custom die support
    dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
    if (!ctx) {
        fprintf(stderr, "Error: failed to create dice context\n");
        return 1;
    }
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            dice_context_destroy(ctx);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("Roll %s - Universal Dice Rolling Library\n", dice_version());
            dice_context_destroy(ctx);
            return 0;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seed") == 0) {
            if (i + 1 < argc) {
                seed = (uint32_t)atol(argv[++i]);
            } else {
                fprintf(stderr, "Error: -s/--seed requires a number\n");
                dice_context_destroy(ctx);
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) {
            if (i + 1 < argc) {
                count = atoi(argv[++i]);
                if (count <= 0) {
                    fprintf(stderr, "Error: count must be positive\n");
                    dice_context_destroy(ctx);
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: -c/--count requires a number\n");
                dice_context_destroy(ctx);
                return 1;
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--individual") == 0) {
            show_individual = 1;
        } else if (strncmp(argv[i], "--die", 5) == 0) {
            const char *definition = NULL;
            if (argv[i][5] == '=') {
                // --die=NAME={...} format
                definition = &argv[i][6];
            } else if (strcmp(argv[i], "--die") == 0 && i + 1 < argc) {
                // --die NAME={...} format
                definition = argv[++i];
            } else {
                fprintf(stderr, "Error: --die requires a definition\n");
                dice_context_destroy(ctx);
                return 1;
            }
            
            if (dice_parse_and_register_die(ctx, definition) != 0) {
                fprintf(stderr, "Error: %s\n", dice_get_error(ctx));
                dice_context_destroy(ctx);
                return 1;
            }
        } else if (argv[i][0] != '-') {
            if (dice_notation == NULL) {
                dice_notation = argv[i];
            } else {
                fprintf(stderr, "Error: multiple dice notations specified\n");
                dice_context_destroy(ctx);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            dice_context_destroy(ctx);
            return 1;
        }
    }
    
    if (dice_notation == NULL) {
        fprintf(stderr, "Error: no dice notation specified\n");
        print_usage(argv[0]);
        dice_context_destroy(ctx);
        return 1;
    }
    
    // Set up RNG with seed
    dice_rng_vtable_t rng = dice_create_system_rng(seed);
    dice_context_set_rng(ctx, &rng);
    
    // Roll dice
    for (int i = 0; i < count; i++) {
        if (show_individual) {
            printf("Roll %d: ", i + 1);
        }
        
        dice_eval_result_t result = dice_roll_expression(ctx, dice_notation);
        if (!result.success) {
            fprintf(stderr, "Error: %s\n", dice_get_error(ctx));
            dice_context_destroy(ctx);
            return 1;
        }
        
        if (count > 1 && !show_individual) {
            printf("Roll %d: %lld\n", i + 1, (long long)result.value);
        } else {
            printf("%lld\n", (long long)result.value);
        }
    }
    
    dice_context_destroy(ctx);
    return 0;
}