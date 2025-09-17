# Roll
A Universal Dice Rolling Library

A comprehensive, cross-platform dice rolling library written in C with bindings for Python, Rust, .NET, and Node.js.

**Current Version**: 2.0.0 - [See Releases](../../releases) for changelog

## Features

- **Fast C Library**: High-performance dice rolling engine
- **Cross-Platform**: Works on Windows, macOS, and Linux  
- **Multiple Build Targets**: Static library, shared library, and console application
- **Standard RPG Notation**: Supports `3d6`, `1d20+5`, `d6`, and mathematical expressions
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
```

### C API Usage

```c
#include "dice.h"

dice_init(12345);  // Optional seed
int result = dice_roll(6);                    // Roll 1d6
int sum = dice_roll_multiple(3, 6);           // Roll 3d6
int total = dice_roll_notation("3d6+2");      // Parse notation
dice_cleanup();
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
