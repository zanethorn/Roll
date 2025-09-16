"""
Test the Python bindings for the Roll dice library
"""

import sys
import os

# Add the bindings directory to the path
sys.path.insert(0, os.path.join(os.path.dirname(__file__)))

import dice

def test_version():
    """Test version function"""
    version = dice.version()
    assert isinstance(version, str)
    assert len(version) > 0
    print(f"Library version: {version}")

def test_initialization():
    """Test library initialization"""
    # Test with seed
    dice.init(12345)
    
    # Test with time-based seed
    dice.init()
    
    # Test with None
    dice.init(None)

def test_single_roll():
    """Test single die roll"""
    dice.init(12345)
    
    # Test valid rolls
    result = dice.roll(6)
    assert 1 <= result <= 6
    
    result = dice.roll(20)
    assert 1 <= result <= 20
    
    # Test invalid rolls
    try:
        dice.roll(0)
        assert False, "Expected DiceError"
    except dice.DiceError:
        pass
    
    try:
        dice.roll(-5)
        assert False, "Expected DiceError"
    except dice.DiceError:
        pass

def test_multiple_rolls():
    """Test multiple dice rolls"""
    dice.init(12345)
    
    # Test valid rolls
    result = dice.roll_multiple(3, 6)
    assert 3 <= result <= 18
    
    result = dice.roll_multiple(1, 20)
    assert 1 <= result <= 20
    
    # Test invalid rolls
    try:
        dice.roll_multiple(0, 6)
        assert False, "Expected DiceError"
    except dice.DiceError:
        pass

def test_individual_rolls():
    """Test individual dice rolls"""
    dice.init(12345)
    
    # Test valid rolls
    sum_result, individual = dice.roll_individual(3, 6)
    assert 3 <= sum_result <= 18
    assert len(individual) == 3
    assert all(1 <= roll <= 6 for roll in individual)
    assert sum(individual) == sum_result
    
    # Test single die
    sum_result, individual = dice.roll_individual(1, 20)
    assert 1 <= sum_result <= 20
    assert len(individual) == 1
    assert individual[0] == sum_result

def test_notation_rolls():
    """Test RPG notation rolls"""
    dice.init(12345)
    
    # Test basic notation
    result = dice.roll_notation("1d6")
    assert 1 <= result <= 6
    
    result = dice.roll_notation("3d6")
    assert 3 <= result <= 18
    
    # Test with modifiers
    result = dice.roll_notation("1d6+5")
    assert 6 <= result <= 11
    
    result = dice.roll_notation("1d6-1")
    assert 0 <= result <= 5
    
    # Test uppercase D
    result = dice.roll_notation("1D6")
    assert 1 <= result <= 6
    
    # Test invalid notation
    try:
        dice.roll_notation("invalid")
        assert False, "Expected DiceError"
    except dice.DiceError:
        pass

def test_class_interface():
    """Test the Dice class interface"""
    d = dice.Dice(12345)
    
    result = d.roll(6)
    assert 1 <= result <= 6
    
    result = d.roll_multiple(3, 6)
    assert 3 <= result <= 18
    
    sum_result, individual = d.roll_individual(3, 6)
    assert sum(individual) == sum_result
    
    result = d.roll_notation("2d6+3")
    assert 5 <= result <= 15

if __name__ == "__main__":
    print("Running Python dice library tests...")
    
    test_version()
    test_initialization()
    test_single_roll()
    test_multiple_rolls()
    test_individual_rolls()
    test_notation_rolls()
    test_class_interface()
    
    print("All Python tests passed!")