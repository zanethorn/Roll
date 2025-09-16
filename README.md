# Roll
A Universal Dice Rolling Library

A comprehensive, cross-platform dice rolling library written in C with bindings for Python, Rust, .NET, and Node.js.

## Features

- **Core C Library**: Fast, reliable dice rolling with comprehensive API
- **Multiple Build Targets**: Static library, shared library (DLL), and console application
- **Language Bindings**: Native bindings for Python, Rust, .NET, and Node.js
- **RPG Notation Support**: Standard dice notation like "3d6", "1d20+5", "2d8-1"
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Unit Tested**: Comprehensive test coverage for all components

## Quick Start

### Building the C Library

```bash
mkdir build && cd build
cmake ..
make

# For shared library (DLL/so)
cmake -DBUILD_SHARED_LIBS=ON ..
make
```

### Using the Console Application

```bash
# Basic dice rolls
./roll 3d6        # Roll 3 six-sided dice
./roll 1d20+5     # Roll d20 with +5 modifier
./roll -c 5 2d8   # Roll 2d8 five times

# With options
./roll -s 12345 3d6    # Use specific seed
./roll --help          # Show all options
```

## Language Bindings

### Python

```python
import sys
sys.path.append('bindings/python')
import dice

dice.init(12345)  # Optional seed
result = dice.roll(6)  # Roll d6
sum_result = dice.roll_multiple(3, 6)  # Roll 3d6
sum_result, individual = dice.roll_individual(4, 6)  # Roll 4d6, get individual results
result = dice.roll_notation("3d6+2")  # RPG notation
```

### Rust

Add to your `Cargo.toml`:
```toml
[dependencies]
roll-dice = { path = "bindings/rust" }
```

```rust
use roll_dice::*;

init(Some(12345));
let result = roll(6)?;  // Roll d6
let sum = roll_multiple(3, 6)?;  // Roll 3d6
let (sum, individual) = roll_individual(4, 6)?;  // Individual results
let result = roll_notation("3d6+2")?;  // RPG notation
```

### .NET

```csharp
using Roll.Dice;

Dice.Init(12345);  // Optional seed
int result = Dice.Roll(6);  // Roll d6
int sum = Dice.RollMultiple(3, 6);  // Roll 3d6
var (sum, individual) = Dice.RollIndividual(4, 6);  // Individual results
int result = Dice.RollNotation("3d6+2");  // RPG notation
```

### Node.js

```javascript
const dice = require('./bindings/node/dice.js');

dice.init(12345);  // Optional seed
const result = dice.roll(6);  // Roll d6
const sum = dice.rollMultiple(3, 6);  // Roll 3d6
const {sum, individual} = dice.rollIndividual(4, 6);  // Individual results
const result = dice.rollNotation("3d6+2");  // RPG notation
```

## API Reference

### Core Functions

- `dice_init(seed)` - Initialize with optional seed
- `dice_roll(sides)` - Roll single die
- `dice_roll_multiple(count, sides)` - Roll multiple dice, return sum
- `dice_roll_individual(count, sides, results)` - Roll multiple dice, get individual results
- `dice_roll_notation(notation)` - Parse and roll RPG notation
- `dice_version()` - Get library version

### Supported Notation

- Basic: `1d6`, `3d6`, `1d20`
- With modifiers: `1d20+5`, `2d8-1`  
- Case insensitive: `1D6` or `1d6`

## Testing

### C Library Tests
```bash
cd build && ctest -V
```

### Language Binding Tests
```bash
# Python
cd bindings/python && python3 test_dice.py

# Rust  
cd bindings/rust && LD_LIBRARY_PATH=../../build cargo test

# .NET
cd bindings/dotnet/Roll.Dice.Tests && dotnet test

# Node.js
cd bindings/node && node test.js
```

## Installation

### System Installation
```bash
cd build
make install  # May require sudo
```

### Development Setup
1. Clone the repository
2. Build the C library: `mkdir build && cd build && cmake .. && make`
3. Test: `ctest -V`
4. Try console app: `./roll 3d6`

## License

MIT License - see LICENSE file for details.

## Contributing

1. Ensure all tests pass
2. Add tests for new features
3. Update documentation
4. Follow existing code style
