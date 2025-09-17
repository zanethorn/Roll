# Dice Notation Grammar

Complete grammar specification for the Roll dice library's notation parsing.

## Overview

The Roll library supports a comprehensive dice notation grammar that includes standard RPG dice notation, mathematical expressions, and advanced dice mechanics. The parser uses recursive descent with operator precedence.

## Current Grammar (Implemented)

### Lexical Tokens

```
NUMBER      := [0-9]+
IDENTIFIER  := [a-zA-Z][a-zA-Z0-9_]*
DICE        := 'd' | 'D'
KEEP_HIGH   := 'kh' | 'KH'  // Keep highest
KEEP_LOW    := 'kl' | 'KL'  // Keep lowest
DROP_HIGH   := 'dh' | 'DH'  // Drop highest
DROP_LOW    := 'dl' | 'DL'  // Drop lowest  
KEEP        := 'k' | 'K'    // Shorthand for keep highest
DROP        := 'd' | 'D'    // Shorthand for drop lowest (context-dependent)
SELECTOR    := 's' | 'S'    // Conditional selection
GT          := '>'          // Greater than
LT          := '<'          // Less than
GTE         := '>='         // Greater than or equal
LTE         := '<='         // Less than or equal
EQ          := '='          // Equal
NEQ         := '<>'         // Not equal
PLUS        := '+'
MINUS       := '-'
MULTIPLY    := '*'
DIVIDE      := '/'
LPAREN      := '('
RPAREN      := ')'
WHITESPACE  := [ \t\n\r]+ (ignored)
```

### Grammar Rules

```
expression := term ((PLUS | MINUS) term)*

term := factor ((MULTIPLY | DIVIDE) factor)*

factor := NUMBER
        | dice_expression  
        | LPAREN expression RPAREN
        | MINUS factor

dice_expression := dice_count? DICE dice_sides (keep_drop_modifier | conditional_modifier)?

keep_drop_modifier := (KEEP_HIGH | KEEP_LOW | DROP_HIGH | DROP_LOW | KEEP | DROP) NUMBER

conditional_modifier := SELECTOR (GT | LT | GTE | LTE | EQ | NEQ) NUMBER

dice_count := NUMBER

dice_sides := NUMBER
```

### Operator Precedence

1. **Unary minus** (highest precedence)
2. **Multiplication** (`*`), **Division** (`/`)
3. **Addition** (`+`), **Subtraction** (`-`) (lowest precedence)

### Implicit Values

- **Dice count**: `d6` is interpreted as `1d6`
- **Dice sides**: Must be explicit (no default)

## Supported Expressions

### Basic Dice Notation

```
1d6         // Single six-sided die
3d6         // Three six-sided dice
1d20        // Single twenty-sided die  
d6          // Implicit 1d6
d20         // Implicit 1d20
```

### Dice with Modifiers

```
1d20+5      // d20 with +5 bonus
2d8-1       // 2d8 with -1 penalty
3d6+2d4     // Mixed dice types
```

### Dice Selection Operations

```
4d6kh3      // Keep highest 3 of 4d6
4d6kl2      // Keep lowest 2 of 4d6
5d6dh2      // Drop highest 2 of 5d6
5d6dl1      // Drop lowest 1 of 5d6
3d6k3       // Shorthand for kh3 (keep highest 3)
8d4d4       // Shorthand for dl4 (drop lowest 4)
4d6K2       // Case insensitive (same as k2)
6d8D3       // Case insensitive (same as d3)
```

### Conditional Selection Operations

```
3d6s>2      // Select all dice greater than 2
4d10s<5     // Select all dice less than 5
5d6s>=4     // Select all dice greater than or equal to 4
6d6s<=3     // Select all dice less than or equal to 3
8d6s=6      // Select all dice equal to 6
4d6s<>1     // Select all dice not equal to 1
3d6S>3      // Case insensitive (same as s>3)
```

### Mathematical Expressions

```
2*3         // Simple multiplication: 6
10/2        // Division: 5  
(2+3)*4     // Parentheses: 20
-5+10       // Unary minus: 5
```

### Complex Expressions

```
2d6+1d4+3           // Mixed dice and constant
(1d6+2)*3           // Dice in parentheses
1d20+5-1d4          // Multiple operations
2*(1d6+3)           // Multiplication with dice
```

### Case Insensitivity

```
1d6    // Standard lowercase
1D6    // Uppercase also works
1d6    // Mixed case acceptable
```

## Parsing Process

### Tokenization

1. Input string is scanned left-to-right
2. Whitespace is ignored
3. Numbers, operators, and keywords are identified
4. Invalid characters cause parse errors

### AST Construction

The parser builds an Abstract Syntax Tree with these node types:

```c
typedef enum {
    DICE_NODE_LITERAL,       // Number literal (42)
    DICE_NODE_BINARY_OP,     // Binary operation (+, -, *, /)
    DICE_NODE_DICE_OP,       // Dice operation (3d6) 
    DICE_NODE_FUNCTION_CALL, // Function call (future)
    DICE_NODE_ANNOTATION     // Metadata (future)
} dice_node_type_t;
```

### Example Parse Trees

**Expression: `3d6+2`**
```
     +
   /   \
  3d6   2
```

**Expression: `2*(1d6+3)`**  
```
     *
   /   \
  2     +
       / \
     1d6  3
```

## Advanced Grammar (Planned)

### Extended Tokens

```
EXCLAMATION := '!'          // Exploding dice
GREATER     := '>'          // Success counting
LESS        := '<'          // Success counting
REROLL      := 'r' | 'R'    // Reroll
FATE        := 'f' | 'F'    // FATE dice
```

### Extended Grammar Rules

```
dice_expression := dice_count? DICE dice_sides modifier*

modifier := exploding_modifier
          | success_modifier
          | reroll_modifier
          | fate_modifier

exploding_modifier := EXCLAMATION (GREATER NUMBER)?

success_modifier := (GREATER | LESS) NUMBER

reroll_modifier := REROLL (GREATER | LESS)? NUMBER

fate_modifier := FATE
```

### Planned Expressions

```
1d6!            // Exploding dice (reroll on 6)
1d6!>4          // Explode on 4, 5, or 6
6d6>4           // Count successes (≥4)
3d6r1           // Reroll 1s
4df             // FATE dice (+1, -1, 0)
```

## Error Handling

### Syntax Errors

Common syntax errors and their messages:

```
"3d"            // Error: Expected dice sides after 'd'
"d"             // Error: Invalid dice expression
"3d6+"          // Error: Expected expression after '+'
"(3d6"          // Error: Unmatched parenthesis
"3d6)"          // Error: Unexpected closing parenthesis
"3d0"           // Error: Dice must have at least 1 side
"0d6"           // Error: Must roll at least 1 die
```

### Semantic Errors

Runtime evaluation errors:

```
"10/0"          // Error: Division by zero
"1000000d6"     // Error: Too many dice (policy limit)
"1d1000000"     // Error: Too many sides (policy limit)
```

## Parser Implementation

### Recursive Descent

The parser uses recursive descent with these key functions:

```c
// Parse entry point
dice_ast_node_t* parse_expression(parser_context_t* ctx);

// Grammar rule functions
dice_ast_node_t* parse_term(parser_context_t* ctx);
dice_ast_node_t* parse_factor(parser_context_t* ctx);
dice_ast_node_t* parse_dice(parser_context_t* ctx);

// Lexical analysis
token_t next_token(parser_context_t* ctx);
void consume_token(parser_context_t* ctx, token_type_t expected);
```

### Error Recovery

The parser attempts to recover from errors by:
1. Reporting specific error messages with position
2. Stopping at safe synchronization points
3. Returning NULL for failed parse attempts

### Memory Management

- Parser uses arena allocation from context
- AST nodes are allocated in context arena
- No manual memory management needed
- Context destruction frees all parse memory

## Usage Examples

### C API Usage

```c
#include "dice.h"

dice_context_t* ctx = dice_context_create(8192, DICE_FEATURE_BASIC);

// Parse expression
dice_ast_node_t* ast = dice_parse(ctx, "3d6+2");
if (ast == NULL) {
    printf("Parse error: %s\n", dice_get_error(ctx));
} else {
    // Evaluate parsed expression
    int result = dice_evaluate(ctx, ast);
    printf("Result: %d\n", result);
}

dice_context_destroy(ctx);
```

### Grammar Validation

Test expressions against the grammar:

```c
// Valid expressions
assert(dice_roll_notation("3d6") >= 0);
assert(dice_roll_notation("1d20+5") >= 0);
assert(dice_roll_notation("(2+3)*4") >= 0);

// Invalid expressions  
assert(dice_roll_notation("3d") < 0);
assert(dice_roll_notation("d") < 0);
assert(dice_roll_notation("3d6+") < 0);
```

## Extending the Grammar

### Adding New Operators

To add new operators:

1. **Add token type** to lexical analyzer
2. **Update grammar rules** for precedence
3. **Add AST node type** if needed
4. **Implement evaluation** for new operations
5. **Add tests** for new syntax

### Operator Precedence Guidelines

When adding operators, follow standard mathematical precedence:
1. Parentheses (highest)
2. Unary operators
3. Multiplication/Division  
4. Addition/Subtraction
5. Comparison operators
6. Logical operators (lowest)

### Example: Adding Exponentiation

```c
// Add token
TOKEN_POWER,  // '^' or '**'

// Update grammar
factor := base_factor (POWER factor)?

// Add AST node
DICE_NODE_POWER,

// Implement evaluation
case DICE_NODE_POWER:
    return pow(left_result, right_result);
```

## Best Practices

### Expression Design

- **Keep it intuitive**: Follow standard RPG notation conventions
- **Be unambiguous**: Avoid syntax that could be interpreted multiple ways
- **Fail fast**: Provide clear error messages for invalid syntax
- **Be consistent**: Use consistent patterns for similar operations

### Error Messages

- **Be specific**: "Expected number after 'd'" vs "Syntax error"
- **Show context**: Include position information when possible
- **Suggest fixes**: "Did you mean '1d6' instead of 'd6'?"
- **Be actionable**: User should know how to fix the error

The grammar is designed to be both powerful and approachable, supporting complex dice mechanics while remaining intuitive for basic use cases.