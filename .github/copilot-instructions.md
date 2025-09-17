# Roll - Universal Dice Rolling Library

Roll is a cross-platform C dice rolling library with bindings for Python, Node.js, Rust, and .NET. Always reference these instructions first and fallback to search or bash commands only when you encounter unexpected information that does not match the info here.

## Working Effectively

### Build System Requirements
- CMake 3.16+ (validated: 3.31.6)
- C99-compatible compiler (validated: GCC 13.3.0)
- Make (validated: 4.3)

### Bootstrap and Build Commands
Build the entire project with these exact commands. NEVER CANCEL these builds.

**Default Static Library Build:**
```bash
cd /home/runner/work/Roll/Roll
mkdir -p build && cd build
cmake ..
make
```
- **Build time: ~15 seconds. Set timeout to 60+ seconds. NEVER CANCEL.**

**Shared Library Build (required for Python/Rust/.NET bindings):**
```bash
cd /home/runner/work/Roll/Roll
rm -rf build && mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
```
- **Build time: ~2 seconds. Set timeout to 30+ seconds. NEVER CANCEL.**

**Development Build (all targets with testing):**
```bash
cd /home/runner/work/Roll/Roll
rm -rf build && mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON -DBUILD_CONSOLE_APP=ON -DBUILD_TESTS=ON ..
make
```

### Testing Commands
Run complete test suite - NEVER CANCEL the test runs:

```bash
cd /home/runner/work/Roll/Roll/build
ctest -V
```
- **Test time: <1 second. Set timeout to 30+ seconds. NEVER CANCEL.**
- **Expected: 11 tests, all should pass**

### Console Application Testing
Always test the console application with these validated commands:

```bash
cd /home/runner/work/Roll/Roll/build
./roll 3d6              # Basic dice roll (output: 3-18)
./roll 1d20+5           # Dice with modifier (output: 6-25)
./roll --ast "3d6+2"    # Show AST structure
./roll --help           # Show full help (comprehensive options)
./roll -c 3 -t 2d6      # Multiple rolls with trace
./roll '4d6kh3'         # Keep highest 3 of 4d6 (ability scores)
./roll '4dF'            # FATE dice (output: -4 to +4)
```

## Language Bindings Testing

### Node.js Bindings ✅ WORKING
Node.js bindings work via subprocess calls to the console application (no shared library needed):

```bash
cd /home/runner/work/Roll/Roll/bindings/node
node test.js
```
- **Test time: <2 seconds**
- **Expected: All tests pass with "🎉 All Node.js tests passed!" message**
- Only requires console application build (not shared library)

### Python Bindings ⚠️ BROKEN
Python bindings currently broken - looking for non-existent `dice_init` function:

```bash
cd /home/runner/work/Roll/Roll/bindings/python
LD_LIBRARY_PATH=../../build python3 test_dice.py
```
- **Status: FAILS with "undefined symbol: dice_init" error**
- **Do not use Python bindings until fixed**

### Rust Bindings ⚠️ BROKEN
Rust bindings currently broken - same `dice_init` issue:

```bash
cd /home/runner/work/Roll/Roll/bindings/rust
LD_LIBRARY_PATH=../../build cargo test
```
- **Status: FAILS with "undefined reference to dice_init" error**
- **Do not use Rust bindings until fixed**

## Validation Scenarios

### CRITICAL: Manual Testing Requirements
After making changes, ALWAYS run through these complete scenarios:

**Console Application Validation:**
1. Build the project successfully
2. Test basic dice rolling: `./roll 3d6` (should output 3-18)
3. Test complex notation: `./roll '2d8+1d6+5'` (should output 8-27)
4. Test AST display: `./roll --ast "3d6+2"` (should show tree structure)
5. Test help system: `./roll --help` (should show comprehensive help)
6. Test custom dice: `./roll '4dF'` (should output -4 to +4)

**Library API Validation:**
1. Build shared library successfully
2. Verify all 11 unit tests pass with `ctest -V`
3. Test Node.js bindings complete scenario

**Build Validation Commands:**
```bash
# Validate static library
file build/libdice.a    # Should show "ar archive"

# Validate shared library  
file build/libdice.so   # Should show "shared object"
ldd build/roll          # Should link correctly

# Validate console app
build/roll --version    # Should show version "2.0.0"
```

## CMake Build Options

| Option | Default | Description | Usage |
|--------|---------|-------------|-------|
| `BUILD_SHARED_LIBS` | `OFF` | Build shared library | Required for Python/Rust/.NET |
| `BUILD_CONSOLE_APP` | `ON` | Build console app | Required for Node.js bindings |
| `BUILD_TESTS` | `ON` | Build unit tests | Always enable for validation |

**Example configurations:**
```bash
# Library only (no console app)
cmake -DBUILD_CONSOLE_APP=OFF ..

# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Custom install prefix
cmake -DCMAKE_INSTALL_PREFIX=/opt/roll ..
```

## Key Code Locations

### Core Implementation
- `src/dice.c` - Simple API functions (`dice_roll`, `dice_roll_notation`)
- `src/parser.c` - Expression parser and AST construction
- `src/eval.c` - AST evaluation and dice rolling logic
- `include/dice.h` - Complete public API (single header)

### Console Application
- `src/cli/roll.c` - Full-featured CLI with comprehensive help

### Tests
- `tests/` - Complete test suite (11 test executables)
- All tests use simple assertions and clear output

### Working Bindings
- `bindings/node/` - Node.js subprocess-based bindings (✅ WORKING)

### Broken Bindings (DO NOT USE)
- `bindings/python/` - Python ctypes bindings (⚠️ BROKEN - dice_init issue)
- `bindings/rust/` - Rust FFI bindings (⚠️ BROKEN - dice_init issue)
- `bindings/dotnet/` - .NET P/Invoke bindings (❓ UNTESTED)

## API Reference

### Simple API (No Context Required)
```c
int dice_roll(int sides);                          // Roll single die
int dice_roll_multiple(int count, int sides);     // Roll multiple dice
int dice_roll_notation(const char *notation);     // Parse and roll expression
const char* dice_version(void);                   // Get version string
```

### Context-Based API (Advanced)
```c
dice_context_t* dice_context_create(size_t arena_size, dice_features_t features);
dice_eval_result_t dice_roll_expression(dice_context_t *ctx, const char *expression);
void dice_context_destroy(dice_context_t *ctx);
```

## Common Errors and Solutions

**Build Errors:**
- Missing CMake: `sudo apt install cmake`
- Missing compiler: `sudo apt install build-essential`

**Runtime Errors:**
- Shared library not found: Set `LD_LIBRARY_PATH=../../build`
- Python/Rust bindings fail: Use Node.js bindings instead (they work)

**Test Failures:**
- If unit tests fail, check that you built with `BUILD_TESTS=ON`
- Tests are probabilistic but very stable - failures indicate real issues

## Development Workflow

1. **Always build and test first:**
   ```bash
   mkdir -p build && cd build && cmake .. && make && ctest -V
   ```

2. **Test console application manually:**
   ```bash
   ./roll 3d6 && ./roll --help
   ```

3. **Test working language bindings:**
   ```bash
   cd ../bindings/node && node test.js
   ```

4. **For shared library development:**
   ```bash
   cmake -DBUILD_SHARED_LIBS=ON .. && make
   ```

5. **Always validate changes with real dice rolling scenarios**

## Repository Structure
```
Roll/
├── src/           - Core C library implementation
├── include/       - Public headers (dice.h)
├── tests/         - Unit test suite (11 tests)
├── bindings/      - Language bindings (Node.js works, others broken)
├── docs/          - Comprehensive documentation
├── build/         - CMake build directory (create with mkdir)
└── CMakeLists.txt - Main build configuration
```

## Important Files
- `README.md` - Project overview and quick start
- `docs/build-guide.md` - Detailed build instructions
- `docs/api-reference.md` - Complete API documentation
- `docs/language-bindings.md` - Language binding guides
- `include/dice.h` - Single public header with all APIs

Always follow this workflow and these exact commands. The build is fast (~15 seconds) and tests are comprehensive (<1 second). Do not skip validation steps.