# Roll 🎲
A Universal Dice Rolling Library

A modern C++ library for dice rolling with support for various dice types, multiple dice rolls, and standard dice notation parsing.

## Features

- **Single Die Rolling**: Roll individual dice with any number of sides
- **Multiple Dice Rolling**: Roll multiple dice at once with sum calculation  
- **Dice Notation Support**: Parse and roll using standard dice notation (e.g., "3d6", "d20")
- **Thread-Safe**: Uses modern C++ random number generation
- **Header-Only Interface**: Easy to integrate into existing projects

## Building

### Requirements

- C++17 compatible compiler
- CMake 3.16 or higher

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/zanethorn/Roll.git
cd Roll

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make

# Run tests
make test

# Run demo
./examples/demo
```

### Build Options

- `BUILD_TESTS`: Enable/disable building tests (default: ON)
- `BUILD_EXAMPLES`: Enable/disable building examples (default: ON)

```bash
cmake -DBUILD_TESTS=OFF -DBUILD_EXAMPLES=OFF ..
```

## Usage

### Basic Usage

```cpp
#include "roll.h"

// Create a single die
roll::Die d6(6);  // 6-sided die
int result = d6.roll();  // Roll once

// Create a dice roller for multiple dice
roll::DiceRoller roller;

// Roll multiple dice
auto results = roller.roll(3, 6);  // Roll 3d6
int sum = roller.rollSum(2, 20);   // Roll 2d20 and get sum

// Use dice notation
auto notation_results = roller.rollFromNotation("4d6");  // Roll 4d6
int notation_sum = roller.rollSumFromNotation("3d8");    // Roll 3d8 and get sum
```

### Supported Dice Notation

- `d6` - Roll a single 6-sided die
- `2d8` - Roll two 8-sided dice  
- `3d10` - Roll three 10-sided dice
- `1d100` - Roll a single 100-sided die

## API Reference

### `roll::Die`

- `Die(int sides = 6)` - Constructor with number of sides
- `int roll()` - Roll the die and return result
- `int getSides()` - Get number of sides

### `roll::DiceRoller`

- `std::vector<int> roll(int count, int sides = 6)` - Roll multiple dice
- `int rollSum(int count, int sides = 6)` - Roll multiple dice and return sum
- `std::vector<int> rollFromNotation(const std::string& notation)` - Parse notation and roll
- `int rollSumFromNotation(const std::string& notation)` - Parse notation, roll, and return sum

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
