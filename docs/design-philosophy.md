# Design Philosophy

The Roll library is built on several core principles that guide its architecture and implementation.

## Core Principles

### 1. Performance and Simplicity

The library prioritizes performance while maintaining a simple, intuitive API. Written in C99, it provides:
- Zero-allocation parsing for most common use cases
- Minimal memory footprint with arena-based allocation
- High-performance random number generation
- Efficient expression evaluation

### 2. Cross-Platform Compatibility

Roll is designed to work consistently across platforms:
- Standard C99 with minimal dependencies
- CMake-based build system for portability
- Consistent behavior across Windows, macOS, and Linux
- Multiple build targets (static library, shared library, console application)

### 3. Extensible Architecture

The codebase is architected for extensions and new features:
- **Abstract Syntax Tree (AST)**: All expressions are parsed to AST nodes, enabling complex transformations
- **Feature Flags**: Controlled through `DICE_FEATURE_*` constants for selective functionality
- **Context-based Design**: Thread-safe contexts support concurrent usage
- **Pluggable RNG**: Custom random number generators can be used via vtables

### 4. Language Binding Flexibility

Multiple approaches are supported for language bindings to maximize compatibility:
- **FFI/Native Bindings**: Direct library linking (Python, Rust, .NET)
- **Subprocess Bindings**: Console application wrapping (Node.js)
- **Consistent API**: Same functionality across all language bindings

### 5. Comprehensive Error Handling

Robust error handling without exceptions:
- **Return Value Conventions**: Negative values indicate errors
- **Context Error State**: Error flags and messages in context objects
- **Graceful Degradation**: Invalid inputs are handled safely
- **Detailed Diagnostics**: Error messages provide actionable information

## Architectural Decisions

### Parser Design

The parser uses recursive descent with operator precedence for reliable and extensible parsing:
- **Token-based Lexing**: Clear separation of lexical analysis and parsing
- **AST Generation**: Expressions become tree structures for evaluation
- **Error Recovery**: Parsing errors are reported with context

### Memory Management

Arena-based allocation provides predictable memory usage:
- **Context Arenas**: Each context has its own memory arena
- **Bulk Deallocation**: Entire contexts can be reset or destroyed at once
- **Pool Allocation**: Efficient allocation of small objects

### Random Number Generation

Pluggable RNG system allows for different quality/performance tradeoffs:
- **System RNG**: Platform-appropriate secure random generation
- **xoshiro256++**: High-performance PRNG for non-cryptographic use
- **Custom RNG**: User-provided random number generators via vtables

### Thread Safety

Context-based design enables concurrent usage:
- **Isolated Contexts**: Each context maintains independent state
- **Global State API**: Simple API uses process-global state for basic use cases
- **No Shared Mutable State**: All contexts are independent

## Future Architecture

The current architecture supports planned advanced features:

### Advanced Dice Notation

The AST design supports complex dice operations:
- **Modifier Nodes**: Keep/drop, exploding dice, rerolls
- **Pool Operations**: Success counting and complex aggregations
- **Conditional Logic**: Future support for conditional dice operations

### Trace System

Built-in tracing provides audit trails:
- **Atomic Operations**: Each die roll is logged
- **Transformation Steps**: Complex operations are broken down
- **Debugging Support**: Detailed traces aid in development and debugging

### Policy Framework

Configurable policies control evaluation:
- **Resource Limits**: Maximum dice counts, evaluation depth
- **Feature Restrictions**: Selective enabling/disabling of features
- **Safety Guards**: Protection against resource exhaustion

## Contributing to the Architecture

When extending the library:
1. **Follow C99 Standards**: Maintain compatibility with older compilers
2. **Use Arena Allocation**: Allocate from context arenas when possible
3. **Add Feature Flags**: New syntax should be gated by feature flags
4. **Maintain AST Structure**: Complex operations should use AST nodes
5. **Test Thoroughly**: All new features require comprehensive test coverage

The architecture is designed to be stable while allowing for significant feature expansion.