# Roll
A Universal Dice Rolling Library

A comprehensive, cross-platform dice rolling library written in C with bindings for Python, Rust, .NET, and Node.js.

**Current Version**: 2.0.0 - [See Releases](../../releases) for changelog

## Features

- **Fast C Library**: High-performance dice rolling engine
- **Cross-Platform**: Works on Windows, macOS, and Linux  
- **Multiple Build Targets**: Static library, shared library, and console application
- **Standard RPG Notation**: Supports `3d6`, `1d20+5`, `d6`, and mathematical expressions
- **AST Visitor Pattern**: Explore and analyze parsed dice expressions
- **Language Bindings**: Python, Node.js, Rust, and .NET support
- **Thread-Safe**: Context-based API for concurrent usage

## Quick Start

### Building

```bash
git clone <repository-url>
cd Roll
mkdir build && cd build
cmake ..
make
```

### Console Usage

```bash
./roll 3d6        # Roll 3 six-sided dice
./roll 1d20+5     # Roll d20 with +5 modifier
./roll "2d8-1"    # Roll 2d8 with -1 penalty
./roll --ast "3d6+2"  # Show AST structure
```

### C API Usage

**Simple API:**
```c
#include "dice.h"

// Simple dice rolling
int result = dice_roll(6);                      // Roll 1d6
int sum = dice_roll_multiple(3, 6);             // Roll 3d6
int total = dice_roll_notation("3d6+2");        // Parse notation
int quick = dice_roll_quick("2d8+1", 12345);   // Roll with seed
```

**Context-Based API (Advanced):**
```c
#include "dice.h"

// Create a context for dice operations
dice_context_t *ctx = dice_context_create(1024 * 1024, DICE_FEATURE_ALL);

// Set custom seed (optional)
dice_rng_vtable_t rng = dice_create_system_rng(12345);
dice_context_set_rng(ctx, &rng);

// Roll dice using expressions
dice_eval_result_t result = dice_roll_expression(ctx, "3d6+2");
if (result.success) {
    printf("Result: %lld\n", (long long)result.value);
}

// Clean up
dice_context_destroy(ctx);
```

The simple API functions create temporary contexts internally for ease of use, while the context-based API provides full control over memory, RNG, and advanced features.

**AST Visitor Pattern (Advanced):**
```c
#include "dice.h"

// Create context and parse expression
dice_context_t *ctx = dice_context_create(64 * 1024, DICE_FEATURE_ALL);
dice_ast_node_t *ast = dice_parse(ctx, "3d6+2");

// Display AST structure using built-in trace visitor
dice_ast_visitor_t trace_visitor = dice_create_trace_visitor(stdout, "  ");
dice_ast_traverse(ast, &trace_visitor);

// Create custom visitor to count nodes
typedef struct { int literals; int dice_ops; } counter_t;
void count_literal(const dice_ast_node_t *node, void *data) {
    ((counter_t*)data)->literals++;
}
void count_dice_op(const dice_ast_node_t *node, void *data) {
    ((counter_t*)data)->dice_ops++;
}

counter_t counter = {0};
dice_ast_visitor_t custom_visitor = {0};
custom_visitor.visit_literal = count_literal;
custom_visitor.visit_dice_op = count_dice_op;
custom_visitor.user_data = &counter;

dice_ast_traverse(ast, &custom_visitor);
printf("Found %d literals and %d dice operations\n", 
       counter.literals, counter.dice_ops);

dice_context_destroy(ctx);
```

### Language Bindings

Choose your preferred language:

**Python** (requires shared library):
```python
import dice
dice.init()
result = dice.roll_notation("3d6+2")
```

**Node.js** (uses console app):
```javascript
const dice = require('./bindings/node/dice.js');
const result = dice.rollNotation("3d6+2");
```

**Rust** (requires shared library):
```rust
use roll_dice::*;
init(None);
let result = roll_notation("3d6+2")?;
```

**.NET** (requires shared library):
```csharp
using Roll.Dice;
Dice.Init();
int result = Dice.RollNotation("3d6+2");
```

## Documentation

Comprehensive documentation is available in the [`docs/`](docs/) directory:

- **[API Reference](docs/api-reference.md)** - Complete C API documentation
- **[Build Guide](docs/build-guide.md)** - Cross-platform build instructions  
- **[Language Bindings](docs/language-bindings.md)** - Integration guides for all languages
- **[Grammar](docs/grammar.md)** - Dice notation syntax specification
- **[AST Visitor Pattern](docs/visitor.md)** - Guide to exploring and analyzing parsed expressions
- **[Troubleshooting](docs/troubleshooting.md)** - Common issues and solutions
- **[Design Philosophy](docs/design-philosophy.md)** - Architecture and design decisions
- **[Changelog](docs/changelog.md)** - Version history and release notes

## Installation

### System Installation
```bash
cd build
make install  # May require sudo
```

### Package Managers
```bash
# Ubuntu/Debian (if packaged)
sudo apt install roll-dice

# Homebrew (if available)
brew install roll-dice
```

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions welcome! See [docs/design-philosophy.md](docs/design-philosophy.md) for architecture details and [docs/build-guide.md](docs/build-guide.md) for development setup.

**Current focus**: Implementing advanced dice notation (`1d6!`, `4d6kh3`, `6d6>4`) - the architecture is ready!

1. Fork and create a feature branch
2. Build and test: `mkdir build && cd build && cmake .. && make && ctest -V`
3. Test language bindings
4. Submit pull request with clear description
