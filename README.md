# Roll
A Universal Dice Rolling Library

A comprehensive, cross-platform dice rolling library written in C with bindings for Python, Rust, .NET, and Node.js.

**Current Version**: 2.0.0 - [See Releases](../../releases) for changelog

## Features

### Core Features ✅
- **Fast C Library**: High-performance dice rolling engine with comprehensive API
- **Multiple Build Targets**: Static library, shared library (DLL), and console application
- **Standard RPG Notation**: Basic dice notation like "3d6", "1d20+5", "2d8-1", "d6" (implicit 1d6)
- **Mathematical Expressions**: Full expression support with parentheses, operators (+, -, *, /)
- **Cross-Platform**: Works on Windows, macOS, and Linux
- **Comprehensive Testing**: Full test coverage for all core components

### Language Bindings ✅
- **Python**: ctypes-based bindings (requires shared library build)
- **Node.js**: Subprocess-based bindings using native console app
- **Rust**: Native FFI bindings with safe wrappers  
- **C#/.NET**: P/Invoke bindings (requires shared library setup)

### Advanced Features ✅
- **Context Management**: Thread-safe contexts for concurrent usage
- **Custom RNG Support**: Pluggable random number generators (system, xoshiro256++)
- **Roll Tracing**: Detailed audit trails for all dice operations
- **Configurable Policies**: Safety limits and evaluation controls
- **Error Handling**: Comprehensive error reporting and recovery

## Future Enhancements 🚧

The following features are planned for future releases. The core architecture supports them, but implementation is in progress:

### Advanced Dice Notation
- **Exploding Dice** 🔄: `1d6!` (explode on max), `1d6!>4` (explode on 4+)
- **Keep/Drop Highest/Lowest** 🔄: `4d6kh3` (keep highest 3), `4d6dl1` (drop lowest 1)  
- **Pool Dice Systems** 🔄: Count successes, target numbers, botches
- **FATE Dice** 🔄: `4df` for FATE/Fudge dice (+, -, blank)

### Extended Features  
- **Dice Pools** 🔄: Complex success counting with target numbers
- **Reroll Mechanics** 🔄: `1d20r1` (reroll 1s), `3d6rr<3` (reroll results < 3)
- **Compound Dice** 🔄: Advanced chaining and conditional rolling
- **Custom Functions** 🔄: User-defined dice manipulation functions

### Integration Enhancements
- **WebAssembly Build** 📋: Browser-compatible dice engine
- **REST API Server** 📋: HTTP service for dice rolling
- **More Language Bindings** 📋: Java, Go, Swift, additional platforms

**Legend**: ✅ Complete | 🔄 In Progress | 📋 Planned

### Contributing to Future Features

The architecture is designed to support these advanced features:
- Feature flags are defined in `include/dice.h` (DICE_FEATURE_* constants)
- AST node types support dice operations with modifiers
- Trace system can log complex transformations
- Parser can be extended with new notation patterns

See the [Contributing](#contributing) section for development guidelines.

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

All language bindings provide the same core functionality as the C API. Choose the approach that works best for your build system:

### Python ✅

**Setup**: Requires shared library build
```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
# Shared library will be at build/libdice.so
```

**Usage**:
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

### Node.js ✅

**Setup**: Uses console application via subprocess (no additional build required)
```bash
# Just build the console app
mkdir build && cd build
cmake ..
make
```

**Usage**:
```javascript
const dice = require('./bindings/node/dice.js');

dice.init(12345);  // Optional seed
const result = dice.roll(6);  // Roll d6
const sum = dice.rollMultiple(3, 6);  // Roll 3d6
const {sum, individual} = dice.rollIndividual(4, 6);  // Individual results
const result = dice.rollNotation("3d6+2");  // RPG notation
```

### Rust ✅

**Setup**: Uses FFI with shared library
```bash
# Build shared library first
mkdir build && cd build  
cmake -DBUILD_SHARED_LIBS=ON ..
make
```

Add to your `Cargo.toml`:
```toml
[dependencies]
roll-dice = { path = "bindings/rust" }
```

**Usage**:
```rust
use roll_dice::*;

init(Some(12345));
let result = roll(6)?;  // Roll d6
let sum = roll_multiple(3, 6)?;  // Roll 3d6
let (sum, individual) = roll_individual(4, 6)?;  // Individual results
let result = roll_notation("3d6+2")?;  // RPG notation
```

### .NET ⚠️

**Setup**: Requires shared library and proper library path
```bash
# Build shared library
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
# Copy libdice.so to your application directory or system library path
```

**Usage**:
```csharp
using Roll.Dice;

Dice.Init(12345);  // Optional seed
int result = Dice.Roll(6);  // Roll d6
int sum = Dice.RollMultiple(3, 6);  // Roll 3d6
var (sum, individual) = Dice.RollIndividual(4, 6);  // Individual results
int result = Dice.RollNotation("3d6+2");  // RPG notation
```

**Note**: .NET binding requires the shared library (`libdice.so`/`libdice.dll`) to be accessible at runtime.

## API Reference

### Basic API (Simple Global State)

These functions use global state and are perfect for simple use cases:

- `dice_init(seed)` - Initialize with optional seed (0 = time-based)
- `dice_roll(sides)` - Roll single die (1 to sides)
- `dice_roll_multiple(count, sides)` - Roll multiple dice, return sum
- `dice_roll_individual(count, sides, results)` - Roll multiple dice, get individual results  
- `dice_roll_notation(notation)` - Parse and roll RPG notation
- `dice_version()` - Get library version string
- `dice_cleanup()` - Cleanup global resources

### Advanced API (Context-Based)

For thread-safety, custom configuration, and advanced features:

**Context Management**:
- `dice_context_create(arena_size, features)` - Create new context
- `dice_context_destroy(ctx)` - Cleanup context  
- `dice_context_reset(ctx)` - Reset for reuse

**Parsing & Evaluation**:
- `dice_parse(ctx, expression)` - Parse expression to AST
- `dice_evaluate(ctx, ast_node)` - Evaluate AST node
- `dice_roll_expression(ctx, expression)` - Parse and evaluate in one call

**Configuration**:
- `dice_context_set_rng(ctx, rng_vtable)` - Set custom RNG
- `dice_context_set_policy(ctx, policy)` - Set evaluation limits

**Tracing & Debugging**:
- `dice_get_trace(ctx)` - Get detailed roll trace
- `dice_clear_trace(ctx)` - Clear trace log
- `dice_has_error(ctx)` - Check for errors
- `dice_get_error(ctx)` - Get error message

### Supported Notation

**Basic Notation** ✅:
- `1d6`, `3d6`, `1d20` - Standard dice
- `1d20+5`, `2d8-1` - With modifiers  
- `d6` - Implicit single die (same as `1d6`)
- `2*3`, `10/2`, `(2+3)*4` - Mathematical expressions
- `2d6+1d4` - Mixed dice expressions
- Case insensitive: `1D6` or `1d6`

**Advanced Notation** 🔄 *(Planned)*:
- `1d6!` - Exploding dice
- `4d6kh3` - Keep highest 3 of 4d6
- `4d6dl1` - Drop lowest 1 of 4d6  
- `6d6>4` - Count successes (≥4)
- `4df` - FATE dice

## Testing

### C Library Tests ✅
```bash
cd build && ctest -V
```

### Language Binding Tests

```bash
# Node.js ✅ (Works out of the box)
cd bindings/node && node test.js

# Python ✅ (Requires shared library build) 
cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make
cd ../bindings/python && python3 test_dice.py

# Rust ✅ (Requires shared library build)
cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make  
cd ../bindings/rust && LD_LIBRARY_PATH=../../build cargo test

# .NET ⚠️ (Requires shared library + path setup)
cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make
# Copy libdice.so to test directory or set LD_LIBRARY_PATH
cd ../bindings/dotnet/Roll.Dice.Tests && dotnet test
```

## Troubleshooting

### Shared Library Issues
If you get "library not found" errors:

**Python/Rust/C#**: These bindings need the shared library:
```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
# Library will be at build/libdice.so (Linux), build/libdice.dll (Windows), build/libdice.dylib (macOS)
```

**Library Path Issues**: If the library isn't found at runtime:
```bash
# Linux/macOS - temporary
export LD_LIBRARY_PATH=/path/to/Roll/build:$LD_LIBRARY_PATH

# Or copy to system location
sudo cp build/libdice.so* /usr/local/lib/
sudo ldconfig  # Linux only
```

### Console App Not Found
If `./roll` command isn't working:
```bash
# Make sure you built it
cd build && cmake .. && make

# Check if it exists
ls -la roll

# Run from build directory
./roll 3d6
```

### Node.js Bindings
Node.js bindings work by calling the console application, so they need the `roll` binary to be built and accessible.

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

We welcome contributions! The current focus is on implementing the advanced dice features outlined in the [Future Enhancements](#future-enhancements-) section.

### Development Guidelines

1. **Build and Test**: Ensure all tests pass
   ```bash
   mkdir build && cd build
   cmake .. && make && ctest -V
   ```

2. **Code Style**: Follow existing C99 style in the codebase

3. **New Features**: 
   - Check `include/dice.h` for feature flag definitions
   - Add tests to `tests/test_dice.c`
   - Update this README's feature status
   - Test with all language bindings

4. **Areas Needing Help**:
   - 🔄 **Exploding Dice**: Parse and evaluate `1d6!` notation
   - 🔄 **Keep/Drop**: Implement `4d6kh3`, `4d6dl1` patterns  
   - 🔄 **Pool Systems**: Success counting for modern RPGs
   - 🔄 **FATE Dice**: `4df` notation support
   - 📋 **More Language Bindings**: Java, Go, Swift wrappers
   - 📋 **WebAssembly**: Browser-compatible builds

### Pull Request Process
1. Fork and create a feature branch
2. Implement feature with tests  
3. Verify all language bindings still work
4. Update documentation
5. Submit pull request with clear description

The codebase is well-architected for extensions - most new dice types just need parser and evaluator additions!
