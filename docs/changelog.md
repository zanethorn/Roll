# Changelog

All notable changes to the Roll dice library are documented here.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/).

## [2.0.0] - Current

**Current Version** - Major architectural improvements and expanded functionality.

### Added
- **Advanced API**: Context-based API for thread safety and advanced features
- **Context Management**: `dice_context_create()`, `dice_context_destroy()`, `dice_context_reset()`
- **AST-based Parser**: Expression parsing to Abstract Syntax Trees
- **Pluggable RNG**: Custom random number generator support via vtables
- **Roll Tracing**: Detailed audit trails with `dice_get_trace()`
- **Policy Framework**: Configurable limits with `dice_context_set_policy()`
- **Error Handling**: Comprehensive error reporting with context state
- **Feature Flags**: Selective enabling of dice notation features
- **Arena Memory Management**: Efficient context-based allocation
- **Extended Testing**: Comprehensive test coverage for all components

### Enhanced
- **Parser Architecture**: Recursive descent parser with operator precedence
- **Expression Evaluation**: Support for complex mathematical expressions
- **Cross-Platform Support**: Improved Windows, macOS, and Linux compatibility
- **Language Bindings**: Enhanced Python, Node.js, Rust, and .NET support
- **Build System**: Improved CMake configuration with multiple targets
- **Documentation**: Comprehensive API documentation and examples

### Performance
- **Zero-Allocation Parsing**: Memory-efficient expression parsing
- **Optimized Evaluation**: Fast AST-based expression evaluation
- **Efficient RNG**: High-performance xoshiro256++ PRNG option
- **Bulk Operations**: Optimized multi-die rolling functions

### Compatibility
- **Backwards Compatible**: All v1.x APIs still supported
- **C99 Standard**: Maintains C99 compatibility for broad compiler support
- **Thread Safety**: New context-based API is fully thread-safe
- **Legacy API**: Deprecated but functional - creates temporary contexts per operation

### Architecture
- **Modular Design**: Separated RNG, memory management, and custom dice into dedicated modules
- **No Static Variables**: Removed all global state from core library
- **Clean Separation**: Clear distinction between public API and internal implementation
- **Stateless Legacy API**: Legacy functions now create temporary contexts for compatibility

---

## [1.0.0] - 2024-12-XX

**Initial Release** - Core dice rolling functionality with basic notation support.

### Added
- **Core Library**: C99 dice rolling engine with comprehensive API
- **Basic Notation**: Standard RPG notation (`3d6`, `1d20+5`, `d6`)
- **Mathematical Expressions**: Full expression support with operators and parentheses
- **Multiple Build Targets**: Static library, shared library, console application
- **Language Bindings**: Initial Python, Node.js, Rust, and .NET support
- **Console Application**: `roll` command-line tool for interactive dice rolling
- **Cross-Platform**: Windows, macOS, and Linux support
- **CMake Build System**: Flexible build configuration

### API Functions
- `dice_init()` - Initialize library with optional seed
- `dice_roll()` - Roll single die
- `dice_roll_multiple()` - Roll multiple dice, return sum
- `dice_roll_individual()` - Roll multiple dice, get individual results
- `dice_roll_notation()` - Parse and roll RPG notation
- `dice_version()` - Get library version
- `dice_cleanup()` - Cleanup resources

### Supported Notation
- Basic dice: `1d6`, `3d6`, `1d20`
- With modifiers: `1d20+5`, `2d8-1`
- Implicit count: `d6` (same as `1d6`)
- Math expressions: `2*3`, `10/2`, `(2+3)*4`
- Mixed expressions: `2d6+1d4+3`
- Case insensitive: `1d6` or `1D6`

### Language Bindings
- **Python**: ctypes-based bindings with shared library
- **Node.js**: Subprocess-based bindings using console app
- **Rust**: Native FFI bindings with safe wrappers
- **.NET**: P/Invoke bindings for C# integration

### Testing
- Comprehensive C library tests with CTest
- Language binding test suites
- Cross-platform CI/CD testing

---

## Planned Releases

### [2.1.0] - Planned

**Advanced Dice Notation** - Implementation of extended dice mechanics.

#### Planned Features
- **Exploding Dice**: `1d6!`, `1d6!>4` notation
- **Keep/Drop Mechanics**: `4d6kh3`, `4d6dl1` patterns
- **Success Counting**: `6d6>4` pool mechanics
- **FATE Dice Support**: `4df` notation
- **Reroll Mechanics**: `1d20r1` patterns

#### Parser Enhancements
- Extended grammar for advanced notation
- Modifier chaining support
- Enhanced error messages
- Performance optimizations

#### API Extensions
- New AST node types for advanced dice
- Enhanced trace information
- Additional policy controls
- Improved error reporting

### [2.2.0] - Planned

**Integration & Performance** - Enhanced integrations and optimization.

#### Planned Features
- **WebAssembly Build**: Browser-compatible dice engine
- **REST API Server**: HTTP service for dice rolling
- **Additional Language Bindings**: Java, Go, Swift support
- **Plugin System**: Custom dice mechanics via plugins

#### Performance Improvements
- SIMD optimizations for bulk operations
- Memory pool optimizations
- Lazy evaluation for complex expressions
- Compilation to bytecode

#### Developer Experience
- Enhanced debugging tools
- Interactive expression tester
- Performance profiling tools
- Documentation generator

---

## Development History

### Architecture Evolution

**Version 1.0**: Simple, direct API with global state
- Single-threaded design
- Basic error handling via return codes
- Minimal memory management
- Direct function calls

**Version 2.0**: Context-based architecture with advanced features
- Thread-safe context design
- Arena-based memory management
- AST-based expression parsing
- Comprehensive error handling
- Pluggable components

**Future Versions**: Advanced dice mechanics and integrations
- Extended notation support
- Performance optimizations
- Additional language targets
- Enhanced developer tools

### API Stability

The Roll library follows semantic versioning:

- **Major versions** (X.0.0): Breaking changes to public APIs
- **Minor versions** (X.Y.0): New features, backwards compatible
- **Patch versions** (X.Y.Z): Bug fixes, no API changes

### Backwards Compatibility

- **Version 2.0**: Maintains full compatibility with 1.x APIs
- **Version 1.x**: All functions remain available and unchanged
- **Global State**: Basic API continues to work as in v1.0
- **Migration Path**: Optional upgrade to context-based API

### Migration Guide

#### From 1.0 to 2.0

The basic API remains unchanged:
```c
// v1.0 code continues to work in v2.0
dice_init(12345);
int result = dice_roll_notation("3d6+2");
dice_cleanup();
```

For new features, use the context API:
```c
// v2.0 advanced features
dice_context_t* ctx = dice_context_create(8192, DICE_FEATURE_ALL);
int result = dice_roll_expression(ctx, "3d6+2");
const dice_trace_t* trace = dice_get_trace(ctx);
dice_context_destroy(ctx);
```

### Breaking Changes

#### Version 2.0
- None (fully backwards compatible)

#### Future Versions
- No breaking changes planned for 2.x series
- Version 3.0 may retire deprecated v1.0 APIs
- Clear migration path will be provided

---

## Contributing

### Changelog Guidelines

When contributing changes:
1. **Add entries** to the "Unreleased" section
2. **Use appropriate categories**: Added, Changed, Deprecated, Removed, Fixed, Security
3. **Include context**: What changed and why
4. **Reference issues**: Link to GitHub issues when applicable
5. **Update on release**: Move entries to versioned sections

### Release Process

1. **Update version** in CMakeLists.txt and relevant files
2. **Move unreleased** changes to new version section
3. **Tag release** with semantic version number
4. **Build and test** all platforms and language bindings
5. **Create GitHub release** with changelog excerpt
6. **Update documentation** with new version information

### Version Support

- **Current version**: Full support and active development
- **Previous major**: Security fixes and critical bugs only
- **Older versions**: Community support, no official maintenance