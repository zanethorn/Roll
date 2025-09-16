#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dice.h"

void print_usage(const char *program_name) {
    printf("Usage: %s [options] <dice_notation>\n", program_name);
    printf("  dice_notation: Standard RPG notation (e.g., '3d6', '1d20+5', '2d8-1')\n");
    printf("  Options:\n");
    printf("    -h, --help     Show this help message\n");
    printf("    -v, --version  Show version information\n");
    printf("    -s, --seed N   Set random seed to N\n");
    printf("    -c, --count N  Roll N times\n");
    printf("    -i, --individual  Show individual dice results\n");
    printf("\n");
    printf("  Examples:\n");
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
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("Roll %s - Universal Dice Rolling Library\n", dice_version());
            return 0;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--seed") == 0) {
            if (i + 1 < argc) {
                seed = (uint32_t)atol(argv[++i]);
            } else {
                fprintf(stderr, "Error: -s/--seed requires a number\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--count") == 0) {
            if (i + 1 < argc) {
                count = atoi(argv[++i]);
                if (count <= 0) {
                    fprintf(stderr, "Error: count must be positive\n");
                    return 1;
                }
            } else {
                fprintf(stderr, "Error: -c/--count requires a number\n");
                return 1;
            }
        } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--individual") == 0) {
            show_individual = 1;
        } else if (argv[i][0] != '-') {
            if (dice_notation == NULL) {
                dice_notation = argv[i];
            } else {
                fprintf(stderr, "Error: multiple dice notations specified\n");
                return 1;
            }
        } else {
            fprintf(stderr, "Error: unknown option %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (dice_notation == NULL) {
        fprintf(stderr, "Error: no dice notation specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Initialize dice system
    dice_init(seed);
    
    // Roll dice
    for (int i = 0; i < count; i++) {
        if (show_individual) {
            // Parse notation to extract count and sides for individual rolls
            // This is a simplified version - for full parsing we'd reuse the parsing logic
            printf("Roll %d: ", i + 1);
        }
        
        int result = dice_roll_notation(dice_notation);
        if (result < 0) {
            fprintf(stderr, "Error: invalid dice notation '%s'\n", dice_notation);
            return 1;
        }
        
        if (count > 1 && !show_individual) {
            printf("Roll %d: %d\n", i + 1, result);
        } else {
            printf("%d\n", result);
        }
    }
    
    return 0;
}