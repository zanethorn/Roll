# Cross-Platform Build Guide

Comprehensive guide for building the Roll dice library on different platforms and configurations.

## Quick Start

### Prerequisites

**All Platforms:**
- CMake 3.16 or later
- C99-compatible compiler (GCC, Clang, MSVC)

**Linux/macOS:**
```bash
# Ubuntu/Debian
sudo apt install build-essential cmake

# macOS with Homebrew  
brew install cmake
```

**Windows:**
- Visual Studio 2019+ with C/C++ workload, OR
- MinGW-w64 with GCC
- CMake (from cmake.org or Visual Studio installer)

### Basic Build

```bash
# Clone and build
git clone <repository-url>
cd Roll
mkdir build && cd build
cmake ..
make                    # Linux/macOS
# OR
cmake --build .         # Cross-platform
```

## Build Configurations

### Static Library (Default)

Creates `libdice.a` (Linux/macOS) or `dice.lib` (Windows):

```bash
mkdir build && cd build
cmake ..
make
```

### Shared Library

Creates `libdice.so` (Linux), `libdice.dylib` (macOS), or `dice.dll` (Windows):

```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
```

### Console Application Only

Skip library, build only the `roll` console application:

```bash
mkdir build && cd build  
cmake -DBUILD_CONSOLE_APP=ON -DBUILD_SHARED_LIBS=OFF ..
make
```

### Development Build

Enable all targets with testing:

```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON -DBUILD_CONSOLE_APP=ON -DBUILD_TESTS=ON ..
make
ctest -V  # Run tests
```

## Platform-Specific Instructions

### Linux

**Ubuntu/Debian:**
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential cmake git

# Build shared library for language bindings
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)

# Install system-wide (optional)
sudo make install
sudo ldconfig
```

**CentOS/RHEL/Fedora:**
```bash
# Install dependencies
sudo dnf install gcc cmake git make           # Fedora
sudo yum install gcc cmake git make           # CentOS/RHEL

# Build process same as Ubuntu
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j$(nproc)
```

### macOS

**Using Xcode Command Line Tools:**
```bash
# Install Xcode tools
xcode-select --install

# Install CMake via Homebrew
brew install cmake

# Build
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make -j$(sysctl -n hw.ncpu)

# Install (optional)
sudo make install
```

**Universal Binaries (Apple Silicon + Intel):**
```bash
mkdir build && cd build
cmake -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" -DBUILD_SHARED_LIBS=ON ..
make
```

### Windows

**Visual Studio (Recommended):**
```cmd
REM Open Visual Studio Developer Command Prompt

mkdir build
cd build
cmake -G "Visual Studio 17 2022" -A x64 ..
cmake --build . --config Release

REM For shared library
cmake -G "Visual Studio 17 2022" -A x64 -DBUILD_SHARED_LIBS=ON ..
cmake --build . --config Release
```

**MinGW-w64:**
```bash
# Using MSYS2 or Git Bash
mkdir build && cd build
cmake -G "MinGW Makefiles" -DBUILD_SHARED_LIBS=ON ..
mingw32-make

# Or use Ninja for faster builds
cmake -G "Ninja" -DBUILD_SHARED_LIBS=ON ..
ninja
```

**Cross-compilation from Linux:**
```bash
# Install mingw-w64 cross compiler
sudo apt install mingw-w64

mkdir build-win && cd build-win
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake ..
make
```

## CMake Options

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_SHARED_LIBS` | `OFF` | Build shared library instead of static |
| `BUILD_CONSOLE_APP` | `ON` | Build the `roll` console application |
| `BUILD_TESTS` | `ON` | Build and enable unit tests |

### Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Build type: Debug, Release, RelWithDebInfo |
| `CMAKE_INSTALL_PREFIX` | System default | Installation prefix |

### Example Configurations

```bash
# Debug build with all features
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_SHARED_LIBS=ON ..

# Release build, library only
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_CONSOLE_APP=OFF ..

# Custom install location
cmake -DCMAKE_INSTALL_PREFIX=/opt/roll ..
```

## Testing

### Running Tests

```bash
cd build
ctest -V                    # Verbose test output
ctest -j$(nproc)            # Parallel test execution
ctest -R dice_tests         # Run specific test pattern
```

### Language Binding Tests

**Node.js (requires console app):**
```bash
cd bindings/node && node test.js
```

**Python (requires shared library):**
```bash
cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make
cd ../bindings/python && python3 test_dice.py
```

**Rust (requires shared library):**
```bash
cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make  
cd ../bindings/rust
LD_LIBRARY_PATH=../../build cargo test          # Linux
DYLD_LIBRARY_PATH=../../build cargo test        # macOS
```

**.NET (requires shared library + setup):**
```bash
cd build && cmake -DBUILD_SHARED_LIBS=ON .. && make
# Copy library to test directory or set library path
cd ../bindings/dotnet/Roll.Dice.Tests && dotnet test
```

## Installation

### System Installation

**Linux/macOS:**
```bash
cd build
sudo make install

# Libraries go to /usr/local/lib
# Headers go to /usr/local/include  
# Binary goes to /usr/local/bin

# Update library cache (Linux only)
sudo ldconfig
```

**Custom Installation Prefix:**
```bash
cmake -DCMAKE_INSTALL_PREFIX=/opt/roll ..
make
sudo make install
```

**Windows:**
```cmd
REM Run as Administrator
cmake --build . --target install
```

### Package Creation

**Creating Distribution Packages:**
```bash
# Source package
make package_source

# Binary package (Linux: DEB/RPM, macOS: Bundle, Windows: ZIP/NSIS)
cpack
```

## Troubleshooting

### Common Build Issues

**CMake Version Too Old:**
```
CMake Error: CMake 3.16 or higher is required
```
Solution: Update CMake or use newer build environment.

**Missing Compiler:**
```
No CMAKE_C_COMPILER could be found
```
Solution: Install build tools for your platform.

**Test Failures:**
```
Test #1: dice_tests .....................***Failed
```
Solution: Check that all source files compiled correctly, run tests individually.

### Platform-Specific Issues

**Linux - Shared Library Not Found:**
```
error while loading shared libraries: libdice.so.1: cannot open shared object
```
Solutions:
```bash
# Temporary fix
export LD_LIBRARY_PATH=/path/to/build:$LD_LIBRARY_PATH

# Permanent fix  
sudo cp build/libdice.so* /usr/local/lib/
sudo ldconfig
```

**macOS - Shared Library Path Issues:**
```
dyld: Library not loaded: libdice.dylib
```
Solutions:
```bash
# Temporary fix
export DYLD_LIBRARY_PATH=/path/to/build:$DYLD_LIBRARY_PATH

# Or install system-wide
sudo make install
```

**Windows - DLL Not Found:**
```
The program can't start because dice.dll is missing
```
Solutions:
- Copy `dice.dll` to application directory
- Add build directory to PATH
- Install system-wide

### Performance Tuning

**Optimized Release Build:**
```bash
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_FLAGS="-O3 -DNDEBUG -flto" ..
```

**Debug Build with Symbols:**
```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_C_FLAGS="-g -O0 -DDICE_DEBUG" ..
```

## Integration with Build Systems

### CMake Integration

Add to your `CMakeLists.txt`:
```cmake
find_package(Roll REQUIRED)
target_link_libraries(your_target Roll::dice)
```

### pkg-config Integration

After installation, use pkg-config:
```bash
gcc $(pkg-config --cflags --libs roll) -o myapp myapp.c
```

### Makefile Integration

```makefile
CFLAGS += $(shell pkg-config --cflags roll)
LDFLAGS += $(shell pkg-config --libs roll)

myapp: myapp.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
```

## Continuous Integration

Example CI configuration files are available in the repository:
- `.github/workflows/ci.yml` - GitHub Actions
- `appveyor.yml` - AppVeyor (Windows)
- `.travis.yml` - Travis CI

These build and test on multiple platforms and compilers automatically.