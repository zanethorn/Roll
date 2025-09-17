#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "dice.h"

// Helper function to parse custom die definition from --die flag
int parse_die_definition(dice_context_t *ctx, const char *definition) {
    // Expected format: NAME={sides...} or NAME=definition
    const char *equals = strchr(definition, '=');
    if (!equals) {
        fprintf(stderr, "Error: --die format should be NAME={definition}, got '%s'\n", definition);
        return -1;
    }
    
    // Extract name
    size_t name_len = equals - definition;
    if (name_len == 0) {
        fprintf(stderr, "Error: --die requires a name before '='\n");
        return -1;
    }
    
    char *name = malloc(name_len + 1);
    strncpy(name, definition, name_len);
    name[name_len] = '\0';
    
    // Parse the definition part using our parser
    const char *def_str = equals + 1;
    
    // Create a temporary dice expression to parse the custom die
    size_t temp_expr_len = strlen(def_str) + 10;
    char *temp_expr = malloc(temp_expr_len);
    snprintf(temp_expr, temp_expr_len, "1d%s", def_str);
    
    // Parse the expression to extract custom die definition
    dice_ast_node_t *ast = dice_parse(ctx, temp_expr);
    free(temp_expr);
    
    if (!ast || ast->type != DICE_NODE_DICE_OP || ast->data.dice_op.dice_type != DICE_DICE_CUSTOM) {
        fprintf(stderr, "Error: invalid custom die definition '%s'\n", def_str);
        free(name);
        return -1;
    }
    
    if (!ast->data.dice_op.custom_die) {
        fprintf(stderr, "Error: could not parse custom die definition '%s'\n", def_str);
        free(name);
        return -1;
    }
    
    // Register the custom die
    const dice_custom_die_t *custom_die = ast->data.dice_op.custom_die;
    int result = dice_register_custom_die(ctx, name, custom_die->sides, custom_die->side_count);
    
    if (result != 0) {
        fprintf(stderr, "Error: failed to register custom die '%s': %s\n", 
                name, dice_get_error(ctx));
    }
    
    free(name);
    return result;
}

void print_usage(const char *program_name) {
    printf("Usage: %s [options] <dice_notation>\n", program_name);
    printf("  dice_notation: Standard RPG notation (e.g., '3d6', '1d20+5', '2d8-1')\n");
    printf("                 or custom dice notation (e.g., '1d{-1,0,1}', '1dF')\n");
    printf("  Options:\n");
    printf("    -h, --help        Show this help message\n");
    printf("    -v, --version     Show version information\n");
    printf("    -s, --seed N      Set random seed to N\n");
    printf("    -c, --count N     Roll N times\n");
    printf("    -t, --trace       Show individual dice results\n");
    printf("    --ast             Show AST (Abstract Syntax Tree) structure\n");
    printf("    --die NAME=DEF    Define a named custom die\n");
    printf("\n");
    printf("  Custom Die Examples:\n");
    printf("    %s '1d{-1,0,1}'                    # Inline FATE die\n", program_name);
    printf("    %s '4dF'                          # FATE dice (auto-registered)\n", program_name);
    printf("    %s --die F={-1,0,1} '4dF'         # Named FATE dice (explicit)\n", program_name);
    printf("    %s --die HQ='{0:\"Skull\",1:\"Shield\"}' '1dHQ'  # Labeled dice\n", program_name);
    printf("    %s '1d{\"Earth\",\"Wind\",\"Fire\"}'      # String-only dice\n", program_name);
    printf("\n");
    printf("  Standard Examples:\n");
    printf("    %s 3d6        # Roll 3 six-sided dice\n", program_name);
    printf("    %s 1d20+5     # Roll 1 twenty-sided die with +5 modifier\n", program_name);
    printf("    %s -c 5 2d8   # Roll 2 eight-sided dice 5 times\n", program_name);
    printf("    %s -t 4d6     # Roll 4 six-sided dice, show individual results\n", program_name);
    printf("    %s --ast '2+3*4'  # Show AST structure for complex expression\n", program_name);
}

int main(int argc, char *argv[]) {
    uint32_t seed = 0;
    int count = 1;
    int show_trace = 0;
    int show_ast = 0;
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
        } else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--trace") == 0) {
            show_trace = 1;
        } else if (strcmp(argv[i], "--ast") == 0) {
            show_ast = 1;
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
            
            if (parse_die_definition(ctx, definition) != 0) {
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
    
    // If AST display is requested, parse and show AST structure
    if (show_ast) {
        dice_ast_node_t *ast = dice_parse(ctx, dice_notation);
        if (!ast) {
            fprintf(stderr, "Error: %s\n", dice_get_error(ctx));
            dice_context_destroy(ctx);
            return 1;
        }
        
        printf("AST structure for '%s':\n", dice_notation);
        dice_ast_visitor_t trace_visitor = dice_create_trace_visitor(stdout, "  ");
        dice_ast_traverse(ast, &trace_visitor);
        printf("\n");
        
        // Don't exit here - continue with rolling if not in AST-only mode
    }
    
    // Roll dice
    for (int i = 0; i < count; i++) {
        // Clear trace for each roll
        dice_clear_trace(ctx);
        
        dice_eval_result_t result = dice_roll_expression(ctx, dice_notation);
        if (!result.success) {
            fprintf(stderr, "Error: %s\n", dice_get_error(ctx));
            dice_context_destroy(ctx);
            return 1;
        }
        
        if (count > 1) {
            printf("Roll %d: %lld\n", i + 1, (long long)result.value);
        } else {
            printf("%lld\n", (long long)result.value);
        }
        
        // Show trace if requested
        if (show_trace) {
            dice_format_trace_stream(ctx, stdout);
        }
    }
    
    dice_context_destroy(ctx);
    return 0;
}