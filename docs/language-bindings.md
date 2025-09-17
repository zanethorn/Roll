# Language Bindings

Complete guide for using Roll dice library with different programming languages.

## Overview

The Roll library provides multiple integration approaches to maximize compatibility across different language ecosystems and build requirements:

- **FFI/Native Bindings**: Direct library linking for optimal performance
- **Subprocess Bindings**: Console application wrapping for easy deployment  
- **Consistent API**: Same functionality across all language bindings

All bindings provide the same core functionality as the C API.

---

## Python ✅

**Fully Implemented** - Uses ctypes for direct shared library integration.

### Setup

Requires shared library build:
```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
# Shared library will be at build/libdice.so (Linux), libdice.dylib (macOS), libdice.dll (Windows)
```

### Installation

```python
import sys
sys.path.append('bindings/python')
import dice
```

### Usage Examples

```python
import dice

# Initialize with seed (optional)
dice.init(12345)  # Or dice.init() for time-based seed

# Basic dice rolling
result = dice.roll(6)                    # Roll 1d6
sum_result = dice.roll_multiple(3, 6)    # Roll 3d6, get sum

# Individual results
sum_result, individual = dice.roll_individual(4, 6)  # Roll 4d6
print(f"Total: {sum_result}")
print(f"Individual: {individual}")       # List of individual die results

# RPG notation  
result = dice.roll_notation("3d6+2")     # Parse and evaluate notation
result = dice.roll_notation("1d20+5")    # d20 with modifier
result = dice.roll_notation("2d8-1")     # Multiple dice with penalty

# Mathematical expressions
result = dice.roll_notation("2*3")       # Simple math: 6
result = dice.roll_notation("(2+3)*4")   # Parentheses: 20
result = dice.roll_notation("2d6+1d4")   # Mixed dice types

# Library information
version = dice.version()
print(f"Library version: {version}")

# Cleanup (optional, done automatically)
dice.cleanup()
```

### Error Handling

```python
result = dice.roll_notation("invalid")
if result == -1:
    print("Error: Invalid dice notation")

# Or use try/except for invalid parameters
try:
    result = dice.roll(0)  # Invalid: 0-sided die
except Exception as e:
    print(f"Error: {e}")
```

### Threading

Python bindings are not thread-safe (use global state). For concurrent usage:
```python
import threading
import dice

# Use locks for thread safety
dice_lock = threading.Lock()

def roll_dice_thread_safe(notation):
    with dice_lock:
        return dice.roll_notation(notation)
```

---

## Node.js ✅

**Fully Implemented** - Uses subprocess calls to console application.

### Setup

Only requires console application (no shared library needed):
```bash
mkdir build && cd build
cmake ..
make
# Console app will be at build/roll
```

### Installation

```javascript
const dice = require('./bindings/node/dice.js');
```

### Usage Examples

```javascript
const dice = require('./bindings/node/dice.js');

// Initialize with seed (optional)
dice.init(12345);  // Or dice.init() for time-based seed

// Basic dice rolling
const result = dice.roll(6);                    // Roll 1d6
const sum = dice.rollMultiple(3, 6);            // Roll 3d6, get sum

// Individual results
const {sum, individual} = dice.rollIndividual(4, 6);  // Roll 4d6
console.log(`Total: ${sum}`);
console.log(`Individual: ${individual}`);       // Array of individual die results

// RPG notation
const result1 = dice.rollNotation("3d6+2");     // Parse and evaluate notation
const result2 = dice.rollNotation("1d20+5");    // d20 with modifier
const result3 = dice.rollNotation("2d8-1");     // Multiple dice with penalty

// Multiple rolls of same notation
const results = dice.rollNotation("3d6", 5);    // Roll 3d6 five times
console.log(`Results: ${results}`);              // Array of 5 results

// Mathematical expressions
const math1 = dice.rollNotation("2*3");         // Simple math: 6
const math2 = dice.rollNotation("(2+3)*4");     // Parentheses: 20  
const mixed = dice.rollNotation("2d6+1d4");     // Mixed dice types

// Library information
const version = dice.version();
console.log(`Library version: ${version}`);
```

### Error Handling

```javascript
try {
    const result = dice.rollNotation("invalid");
} catch (error) {
    console.log(`Error: ${error.message}`);
}

// Or check for error results
const result = dice.roll(-1);  // Invalid parameter
if (result === null) {
    console.log("Error occurred");
}
```

### Async Usage

For non-blocking operations in Node.js applications:
```javascript
const util = require('util');
const dice = require('./bindings/node/dice.js');

// Promisify for async/await
const rollAsync = util.promisify((notation, callback) => {
    setImmediate(() => {
        try {
            const result = dice.rollNotation(notation);
            callback(null, result);
        } catch (error) {
            callback(error);
        }
    });
});

async function example() {
    try {
        const result = await rollAsync("3d6+2");
        console.log(`Result: ${result}`);
    } catch (error) {
        console.log(`Error: ${error.message}`);
    }
}
```

---

## Rust ✅

**Fully Implemented** - Uses FFI with safe Rust wrappers.

### Setup

Requires shared library build:
```bash
mkdir build && cd build  
cmake -DBUILD_SHARED_LIBS=ON ..
make
```

### Installation

Add to your `Cargo.toml`:
```toml
[dependencies]
roll-dice = { path = "bindings/rust" }
```

### Usage Examples

```rust
use roll_dice::*;

fn main() -> Result<(), DiceError> {
    // Initialize with seed (optional)
    init(Some(12345))?;  // Or init(None) for time-based seed

    // Basic dice rolling
    let result = roll(6)?;                    // Roll 1d6
    let sum = roll_multiple(3, 6)?;           // Roll 3d6, get sum
    
    // Individual results
    let (sum, individual) = roll_individual(4, 6)?;  // Roll 4d6
    println!("Total: {}", sum);
    println!("Individual: {:?}", individual); // Vec<i32> of results

    // RPG notation
    let result1 = roll_notation("3d6+2")?;    // Parse and evaluate notation
    let result2 = roll_notation("1d20+5")?;   // d20 with modifier
    let result3 = roll_notation("2d8-1")?;    // Multiple dice with penalty

    // Mathematical expressions
    let math1 = roll_notation("2*3")?;        // Simple math: 6
    let math2 = roll_notation("(2+3)*4")?;    // Parentheses: 20
    let mixed = roll_notation("2d6+1d4")?;    // Mixed dice types

    // Library information
    let version = version();
    println!("Library version: {}", version);

    Ok(())
}
```

### Error Handling

```rust
use roll_dice::{*, DiceError};

fn safe_rolling() -> Result<i32, DiceError> {
    init(Some(12345))?;
    
    // All operations return Results
    let result = roll_notation("3d6+2")?;
    
    // Handle specific errors
    match roll_notation("invalid") {
        Ok(result) => println!("Result: {}", result),
        Err(DiceError::ParseError(msg)) => println!("Parse error: {}", msg),
        Err(DiceError::InvalidParameter(msg)) => println!("Invalid parameter: {}", msg),
        Err(e) => println!("Other error: {}", e),
    }
    
    Ok(result)
}
```

### Threading

Each thread should use separate contexts for thread safety:
```rust
use std::thread;
use roll_dice::*;

fn concurrent_rolling() {
    let handles: Vec<_> = (0..4).map(|thread_id| {
        thread::spawn(move || -> Result<i32, DiceError> {
            init(Some(12345 + thread_id as u32))?;
            roll_notation("3d6")
        })
    }).collect();

    for (i, handle) in handles.into_iter().enumerate() {
        match handle.join().unwrap() {
            Ok(result) => println!("Thread {}: {}", i, result),
            Err(e) => println!("Thread {} error: {}", i, e),
        }
    }
}
```

### Advanced Usage

Using context API (when available):
```rust
use roll_dice::context::*;

fn advanced_usage() -> Result<(), DiceError> {
    let ctx = DiceContext::new(8192, FeatureFlags::ALL)?;
    
    let result = ctx.roll_expression("3d6+2")?;
    let trace = ctx.get_trace();
    
    println!("Result: {}", result);
    println!("Trace: {:?}", trace);
    
    Ok(())
}
```

---

## .NET ⚠️

**Implemented with Setup Requirements** - Uses P/Invoke with shared library.

### Setup

Requires shared library and proper library path configuration:
```bash
mkdir build && cd build
cmake -DBUILD_SHARED_LIBS=ON ..
make
# Copy libdice.so/libdice.dll to your application directory or system library path
```

### Installation

Add project reference or NuGet package:
```xml
<ProjectReference Include="../bindings/dotnet/Roll.Dice/Roll.Dice.csproj" />
```

### Usage Examples

```csharp
using Roll.Dice;

class Program 
{
    static void Main(string[] args)
    {
        try 
        {
            // Initialize with seed (optional)
            Dice.Init(12345);  // Or Dice.Init() for time-based seed

            // Basic dice rolling
            int result = Dice.Roll(6);                    // Roll 1d6
            int sum = Dice.RollMultiple(3, 6);            // Roll 3d6, get sum

            // Individual results
            var (total, individual) = Dice.RollIndividual(4, 6);  // Roll 4d6
            Console.WriteLine($"Total: {total}");
            Console.WriteLine($"Individual: [{string.Join(", ", individual)}]");

            // RPG notation
            int result1 = Dice.RollNotation("3d6+2");     // Parse and evaluate notation
            int result2 = Dice.RollNotation("1d20+5");    // d20 with modifier
            int result3 = Dice.RollNotation("2d8-1");     // Multiple dice with penalty

            // Mathematical expressions
            int math1 = Dice.RollNotation("2*3");         // Simple math: 6
            int math2 = Dice.RollNotation("(2+3)*4");     // Parentheses: 20
            int mixed = Dice.RollNotation("2d6+1d4");     // Mixed dice types

            // Library information
            string version = Dice.Version();
            Console.WriteLine($"Library version: {version}");
        }
        catch (DiceException ex)
        {
            Console.WriteLine($"Dice error: {ex.Message}");
        }
        catch (Exception ex)
        {
            Console.WriteLine($"General error: {ex.Message}");
        }
    }
}
```

### Error Handling

```csharp
using Roll.Dice;

try 
{
    int result = Dice.RollNotation("invalid notation");
}
catch (DiceException ex)
{
    Console.WriteLine($"Dice parsing error: {ex.Message}");
}
catch (DllNotFoundException)
{
    Console.WriteLine("Dice library not found. Ensure libdice.dll/.so is in your path.");
}
```

### Library Path Configuration

**Windows:**
```cmd
REM Copy DLL to application directory
copy build\dice.dll MyApp\bin\Debug\

REM Or add to system PATH
set PATH=%PATH%;C:\path\to\build
```

**Linux:**
```bash
# Copy to application directory
cp build/libdice.so MyApp/bin/Debug/

# Or set library path
export LD_LIBRARY_PATH=/path/to/build:$LD_LIBRARY_PATH

# Or install system-wide
sudo cp build/libdice.so* /usr/local/lib/
sudo ldconfig
```

**macOS:**
```bash
# Copy to application directory  
cp build/libdice.dylib MyApp/bin/Debug/

# Or set library path
export DYLD_LIBRARY_PATH=/path/to/build:$DYLD_LIBRARY_PATH
```

### Threading

.NET bindings use global state and are not thread-safe:
```csharp
using System.Threading;
using Roll.Dice;

private static readonly object DiceLock = new object();

public static int ThreadSafeRoll(string notation)
{
    lock (DiceLock)
    {
        return Dice.RollNotation(notation);
    }
}
```

---

## Future Language Bindings 📋

Planned language support for future releases:

### Java
- **JNI bindings** for direct native integration
- **Maven/Gradle** build integration  
- **Android support** for mobile applications

### Go
- **CGO bindings** for direct library integration
- **Go modules** for easy dependency management
- **Cross-compilation** support

### Swift
- **Swift Package Manager** integration
- **iOS/macOS** native support
- **Bridging headers** for C interoperability

### WebAssembly  
- **Emscripten build** for browser compatibility
- **JavaScript bindings** for web applications
- **npm package** for easy installation

---

## Integration Patterns

### Build System Integration

**CMake:**
```cmake
find_package(Roll REQUIRED)
target_link_libraries(your_target Roll::dice)
```

**pkg-config:**
```bash
gcc $(pkg-config --cflags --libs roll) -o myapp myapp.c
```

**Python setup.py:**
```python
from setuptools import setup, Extension

dice_extension = Extension(
    'dice',
    sources=['dice_wrapper.c'],
    libraries=['dice'],
    library_dirs=['/usr/local/lib']
)

setup(ext_modules=[dice_extension])
```

### Deployment Strategies

**Embedded Library:**
- Bundle shared library with application
- No external dependencies
- Larger application size

**System Installation:**  
- Install library system-wide
- Smaller application size
- Requires installation privileges

**Subprocess Approach:**
- Bundle console application
- No library path issues
- Slight performance overhead

## Testing Language Bindings

### Automated Testing

All language bindings include comprehensive test suites:

```bash
# Node.js tests
cd bindings/node && node test.js

# Python tests (requires shared library)
cd bindings/python && python3 test_dice.py

# Rust tests (requires shared library)
cd bindings/rust && LD_LIBRARY_PATH=../../build cargo test

# .NET tests (requires shared library in path)
cd bindings/dotnet/Roll.Dice.Tests && dotnet test
```

### Cross-Platform Testing

CI/CD systems test all bindings across:
- **Windows**: Visual Studio 2019+, MinGW
- **macOS**: Xcode Command Line Tools, Homebrew
- **Linux**: GCC, Clang on Ubuntu/CentOS

### Performance Testing

Benchmark results across languages (approximate):
- **C API**: ~1,000,000 dice/second (baseline)
- **Rust FFI**: ~900,000 dice/second (10% overhead)
- **Python ctypes**: ~100,000 dice/second (ctypes overhead)
- **Node.js subprocess**: ~1,000 dice/second (subprocess overhead)
- **.NET P/Invoke**: ~500,000 dice/second (P/Invoke overhead)

Choose binding based on your performance requirements and deployment constraints.