# Troubleshooting Guide

Common issues and solutions for the Roll dice library across different platforms and language bindings.

## Build Issues

### CMake Configuration Problems

**CMake Version Too Old**
```
CMake Error: CMake 3.16 or higher is required. You are running version 3.10.2
```
**Solution:**
```bash
# Ubuntu/Debian - add CMake PPA
sudo apt remove cmake
sudo apt install software-properties-common
sudo add-apt-repository ppa:kitware/cmake-updates
sudo apt update && sudo apt install cmake

# CentOS/RHEL - use EPEL repository
sudo yum install epel-release
sudo yum install cmake3
sudo ln -s /usr/bin/cmake3 /usr/local/bin/cmake

# macOS - use Homebrew
brew install cmake

# Windows - download from cmake.org
```

**Compiler Not Found**
```
No CMAKE_C_COMPILER could be found.
```
**Solution:**
```bash
# Ubuntu/Debian
sudo apt install build-essential

# CentOS/RHEL/Fedora  
sudo dnf install gcc make               # Fedora
sudo yum install gcc make               # CentOS/RHEL

# macOS
xcode-select --install

# Windows - install Visual Studio with C++ workload or MinGW
```

### Build Failures

**Make/Ninja Build Errors**
```
make: *** No targets specified and no makefile found. Stop.
```
**Solution:**
```bash
# Ensure you're in the build directory
mkdir build && cd build
cmake ..
make

# Or use cmake to build (cross-platform)
cmake --build .
```

**Missing Header Files**
```
fatal error: dice.h: No such file or directory
```
**Solution:**
- Ensure `include/` directory exists in project root
- Check CMake include directories configuration
- Verify all source files are in `src/` directory

### Test Failures

**Test Executable Not Found**
```
Could not find executable /path/to/Roll/build/tests/test_dice
```
**Solution:**
```bash
# Ensure tests are enabled
cmake -DBUILD_TESTS=ON ..
make

# Check if test executable was built
ls -la tests/test_dice

# Run tests manually
./tests/test_dice
```

**Random Test Failures**
```
FAIL: dice_roll(6) returned 0, expected 1-6
```
**Solution:**
- Usually indicates RNG initialization issues
- Check that `dice_init()` is called before testing
- Verify system entropy sources are available

---

## Library Loading Issues

### Shared Library Not Found

**Linux Error:**
```
error while loading shared libraries: libdice.so.1: cannot open shared object file
```
**Solutions:**
```bash
# Temporary fix - set library path
export LD_LIBRARY_PATH=/path/to/Roll/build:$LD_LIBRARY_PATH

# Permanent fix - install system-wide
sudo cp build/libdice.so* /usr/local/lib/
sudo ldconfig

# Or add to system library path
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/roll.conf
sudo ldconfig

# Verify library is found
ldd your_application  # Should show libdice.so found
```

**macOS Error:**
```
dyld: Library not loaded: libdice.dylib
Reason: image not found
```
**Solutions:**
```bash
# Temporary fix
export DYLD_LIBRARY_PATH=/path/to/Roll/build:$DYLD_LIBRARY_PATH

# Permanent fix - install system-wide
sudo cp build/libdice.dylib /usr/local/lib/

# Or use install_name_tool to fix paths
install_name_tool -change libdice.dylib @rpath/libdice.dylib your_app

# Verify with otool
otool -L your_application
```

**Windows Error:**
```
The program can't start because dice.dll is missing from your computer
```
**Solutions:**
```cmd
REM Copy DLL to application directory
copy build\dice.dll myapp\

REM Or add to system PATH
set PATH=%PATH%;C:\path\to\Roll\build

REM For permanent PATH addition
setx PATH "%PATH%;C:\path\to\Roll\build"

REM Verify DLL dependencies
dumpbin /dependents your_application.exe
```

### Console Application Issues

**Roll Command Not Found**
```
bash: ./roll: No such file or directory
```
**Solutions:**
```bash
# Ensure console app is built
cmake -DBUILD_CONSOLE_APP=ON ..
make

# Check if executable exists
ls -la roll

# Check permissions
chmod +x roll

# Run from build directory
cd build && ./roll 3d6
```

**Permission Denied**
```
bash: ./roll: Permission denied
```
**Solution:**
```bash
chmod +x roll
```

---

## Language Binding Issues

### Python Binding Problems

**ctypes Library Loading Error**
```python
OSError: /path/to/libdice.so: cannot open shared object file
```
**Solutions:**
```python
# Check library path in Python
import ctypes
print(ctypes.util.find_library('dice'))

# Set library path explicitly
import sys
sys.path.append('/path/to/Roll/build')

# Or modify dice.py to use absolute path
lib = ctypes.CDLL('/absolute/path/to/libdice.so')
```

**Function Not Found Error**
```python
AttributeError: function 'dice_roll' not found
```
**Solutions:**
- Ensure shared library is built with all symbols exported
- Check library was built with same architecture (32-bit vs 64-bit)
- Verify function names haven't been mangled

**Wrong Architecture**
```python
OSError: /path/to/libdice.so: wrong ELF class: ELFCLASS64
```
**Solution:**
```bash
# Rebuild library for correct architecture
mkdir build32 && cd build32
cmake -DCMAKE_C_FLAGS=-m32 ..
make

# Or use 64-bit Python if library is 64-bit
python3 test_dice.py
```

### Node.js Binding Problems

**Console App Not Found**
```javascript
Error: spawn roll ENOENT
```
**Solutions:**
```javascript
// Check that console app is built
// Set explicit path in dice.js
const rollPath = path.join(__dirname, '../../build/roll');

// Or ensure build directory is in PATH
process.env.PATH += path.delimiter + path.join(__dirname, '../../build');
```

**Subprocess Timeout**
```javascript
Error: Command failed: roll 3d6 (timeout)
```
**Solutions:**
- Check if console app is hanging
- Increase timeout in dice.js
- Test console app directly: `./build/roll 3d6`

### Rust Binding Problems

**Link Error**
```
= note: /usr/bin/ld: cannot find -ldice
```
**Solutions:**
```bash
# Set library path for Rust builds
export LD_LIBRARY_PATH=/path/to/Roll/build:$LD_LIBRARY_PATH
cargo test

# Or copy library to Rust's library search path
cp build/libdice.so /usr/local/lib/

# Or use build script in Cargo.toml
# build.rs:
println!("cargo:rustc-link-search=/path/to/Roll/build");
```

**Symbol Not Found**
```
undefined symbol: dice_roll
```
**Solutions:**
- Ensure shared library is built and up-to-date
- Check that library exports expected symbols: `nm -D libdice.so | grep dice`
- Verify Rust FFI declarations match C function signatures

### .NET Binding Problems

**DllNotFoundException**
```csharp
System.DllNotFoundException: Unable to load DLL 'dice' or one of its dependencies
```
**Solutions:**
```csharp
// Windows - copy DLL to application directory
// Linux - set LD_LIBRARY_PATH or install system-wide  
// macOS - set DYLD_LIBRARY_PATH

// Or specify absolute path in DllImport
[DllImport("/absolute/path/to/libdice.so")]

// Check DLL architecture matches .NET app (x64 vs x86)
```

**EntryPointNotFoundException**
```csharp
System.EntryPointNotFoundException: Unable to find an entry point named 'dice_roll'
```
**Solutions:**
- Verify function name spelling in DllImport attributes
- Check that shared library exports expected functions
- Ensure calling convention matches (default vs cdecl)

---

## Runtime Issues

### Dice Rolling Anomalies

**Always Returns Same Value**
```c
// Multiple calls return identical results
int r1 = dice_roll(6);  // returns 4
int r2 = dice_roll(6);  // returns 4 (same!)
```
**Solutions:**
```c
// Ensure dice_init() is called with proper seed
dice_init(0);  // Use time-based seed
// Or use different seed each time
dice_init(time(NULL));
```

**Invalid Results**
```c
int result = dice_roll(6);  // returns 0 or 7
```
**Solutions:**
- Check for integer overflow in calculations
- Verify dice parameters are valid (positive number of sides)
- Ensure random number generator is properly initialized

**Parsing Failures**
```c
int result = dice_roll_notation("3d6");  // returns -1
```
**Solutions:**
```c
// Use advanced API to get detailed error information
dice_context_t* ctx = dice_context_create(8192, DICE_FEATURE_BASIC);
int result = dice_roll_expression(ctx, "3d6");
if (dice_has_error(ctx)) {
    printf("Parse error: %s\n", dice_get_error(ctx));
}
dice_context_destroy(ctx);
```

### Memory Issues

**Segmentation Faults**
```
Segmentation fault (core dumped)
```
**Debug Steps:**
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..
make

# Run with debugger
gdb ./your_program
(gdb) run
(gdb) bt  # Show backtrace when it crashes

# Check with valgrind
valgrind --tool=memcheck --leak-check=full ./your_program
```

**Memory Leaks**
```
==12345== LEAK SUMMARY:
==12345==    definitely lost: 1,024 bytes in 10 blocks
```
**Solutions:**
- Ensure `dice_cleanup()` is called for basic API
- Ensure `dice_context_destroy()` is called for each context
- Check that all allocated memory is properly freed

### Performance Issues

**Slow Dice Rolling**
```c
// Taking too long for simple operations
clock_t start = clock();
int result = dice_roll_notation("3d6");
clock_t end = clock();
printf("Time: %f seconds\n", ((double)(end - start)) / CLOCKS_PER_SEC);
```
**Solutions:**
- Use basic API for simple operations (avoids parsing overhead)
- Cache parsed expressions with advanced API
- Consider bulk operations for multiple dice
- Profile with tools like `perf` or `gprof`

---

## Platform-Specific Issues

### Windows Specific

**Visual Studio Build Warnings**
```
warning C4996: 'strcpy': This function may be unsafe
```
**Solution:**
```c
// Add to CMakeLists.txt or compiler flags
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
```

**MinGW Compilation Issues**
```
undefined reference to `__imp_CreateThread'
```
**Solution:**
```bash
# Link with required Windows libraries
cmake -G "MinGW Makefiles" -DCMAKE_C_FLAGS="-lwinpthread" ..
```

### macOS Specific

**Apple Silicon Compatibility**
```
Bad CPU type in executable
```
**Solution:**
```bash
# Build universal binary
cmake -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" ..
make

# Or build for specific architecture
cmake -DCMAKE_OSX_ARCHITECTURES=arm64 ..    # Apple Silicon
cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 ..  # Intel
```

**Homebrew Library Conflicts**
```
dyld: Symbol not found: _dice_roll
```
**Solution:**
```bash
# Use Homebrew's libraries consistently
brew install cmake
export PATH="/usr/local/bin:$PATH"  # Prioritize Homebrew
```

### Linux Specific

**Old glibc Compatibility**
```
./roll: /lib64/libc.so.6: version `GLIBC_2.17' not found
```
**Solution:**
- Build on older system with compatible glibc
- Use static linking: `cmake -DBUILD_SHARED_LIBS=OFF ..`
- Build with musl libc for broader compatibility

---

## Getting Help

### Diagnostic Information

When reporting issues, include:

```bash
# System information
uname -a                    # OS and architecture
gcc --version              # Compiler version
cmake --version            # CMake version

# Library information
./roll --version           # Library version
ldd ./roll                 # Shared library dependencies (Linux)
otool -L ./roll            # Shared library dependencies (macOS)

# Build configuration
cat CMakeCache.txt | grep CMAKE_BUILD_TYPE
```

### Debug Builds

```bash
# Build with debugging enabled
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug -DDICE_DEBUG=ON ..
make

# Run with verbose output
./roll --verbose 3d6
```

### Community Resources

- **GitHub Issues**: Report bugs and feature requests
- **Documentation**: Check docs/ directory for detailed guides
- **Examples**: Look at language binding test files for usage examples
- **Source Code**: Read the implementation for understanding behavior

### Professional Support

For commercial applications requiring guaranteed support:
- Review the LICENSE file for terms
- Consider professional support contracts
- Engage with maintainers for custom integrations