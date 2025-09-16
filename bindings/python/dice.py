"""
Python bindings for the Roll dice library using ctypes.
"""

import ctypes
import os
from pathlib import Path
from typing import List, Optional

# Find the shared library
def _find_library():
    """Find the dice shared library"""
    current_dir = Path(__file__).parent.parent.parent
    
    # Look in common library locations
    possible_paths = [
        current_dir / "build" / "libdice.so",
        current_dir / "build" / "libdice.so.1", 
        current_dir / "build" / "libdice.dll", 
        current_dir / "build" / "libdice.dylib",
        "/usr/local/lib/libdice.so",
        "/usr/lib/libdice.so"
    ]
    
    for path in possible_paths:
        if Path(path).exists():
            return str(path)
    
    raise RuntimeError("Could not find dice library. Please build the C library first.")

# Load the library
_lib_path = _find_library()
_lib = ctypes.CDLL(_lib_path)

# Define function signatures
_lib.dice_init.argtypes = [ctypes.c_uint32]
_lib.dice_init.restype = None

_lib.dice_roll.argtypes = [ctypes.c_int]
_lib.dice_roll.restype = ctypes.c_int

_lib.dice_roll_multiple.argtypes = [ctypes.c_int, ctypes.c_int]
_lib.dice_roll_multiple.restype = ctypes.c_int

_lib.dice_roll_individual.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.POINTER(ctypes.c_int)]
_lib.dice_roll_individual.restype = ctypes.c_int

_lib.dice_roll_notation.argtypes = [ctypes.c_char_p]
_lib.dice_roll_notation.restype = ctypes.c_int

_lib.dice_version.argtypes = []
_lib.dice_version.restype = ctypes.c_char_p

class DiceError(Exception):
    """Exception raised for dice library errors"""
    pass

class Dice:
    """Python interface to the Roll dice library"""
    
    def __init__(self, seed: Optional[int] = None):
        """Initialize the dice library
        
        Args:
            seed: Random seed (None for time-based seed)
        """
        if seed is None:
            seed = 0
        _lib.dice_init(ctypes.c_uint32(seed))
    
    @staticmethod
    def version() -> str:
        """Get the library version"""
        return _lib.dice_version().decode('utf-8')
    
    @staticmethod
    def roll(sides: int) -> int:
        """Roll a single die
        
        Args:
            sides: Number of sides on the die
            
        Returns:
            Random value between 1 and sides (inclusive)
            
        Raises:
            DiceError: If sides <= 0
        """
        result = _lib.dice_roll(ctypes.c_int(sides))
        if result == -1:
            raise DiceError(f"Invalid number of sides: {sides}")
        return result
    
    @staticmethod
    def roll_multiple(count: int, sides: int) -> int:
        """Roll multiple dice and return the sum
        
        Args:
            count: Number of dice to roll
            sides: Number of sides on each die
            
        Returns:
            Sum of all dice rolls
            
        Raises:
            DiceError: If count <= 0 or sides <= 0
        """
        result = _lib.dice_roll_multiple(ctypes.c_int(count), ctypes.c_int(sides))
        if result == -1:
            raise DiceError(f"Invalid parameters: count={count}, sides={sides}")
        return result
    
    @staticmethod
    def roll_individual(count: int, sides: int) -> tuple[int, List[int]]:
        """Roll multiple dice and return individual results
        
        Args:
            count: Number of dice to roll
            sides: Number of sides on each die
            
        Returns:
            Tuple of (sum, list_of_individual_results)
            
        Raises:
            DiceError: If count <= 0 or sides <= 0
        """
        if count <= 0 or sides <= 0:
            raise DiceError(f"Invalid parameters: count={count}, sides={sides}")
        
        # Create array for results
        results_array = (ctypes.c_int * count)()
        
        sum_result = _lib.dice_roll_individual(
            ctypes.c_int(count), 
            ctypes.c_int(sides), 
            results_array
        )
        
        if sum_result == -1:
            raise DiceError(f"Invalid parameters: count={count}, sides={sides}")
        
        # Convert to Python list
        results = [results_array[i] for i in range(count)]
        
        return sum_result, results
    
    @staticmethod
    def roll_notation(notation: str) -> int:
        """Roll dice using RPG notation
        
        Args:
            notation: Dice notation like "3d6", "1d20+5", "2d8-1"
            
        Returns:
            Result of the dice roll
            
        Raises:
            DiceError: If notation is invalid
        """
        notation_bytes = notation.encode('utf-8')
        result = _lib.dice_roll_notation(notation_bytes)
        if result == -1:
            raise DiceError(f"Invalid dice notation: {notation}")
        return result

# Convenience functions
def init(seed: Optional[int] = None):
    """Initialize the dice library"""
    Dice(seed)

def version() -> str:
    """Get the library version"""
    return Dice.version()

def roll(sides: int) -> int:
    """Roll a single die"""
    return Dice.roll(sides)

def roll_multiple(count: int, sides: int) -> int:
    """Roll multiple dice and return the sum"""
    return Dice.roll_multiple(count, sides)

def roll_individual(count: int, sides: int) -> tuple[int, List[int]]:
    """Roll multiple dice and return individual results"""
    return Dice.roll_individual(count, sides)

def roll_notation(notation: str) -> int:
    """Roll dice using RPG notation"""
    return Dice.roll_notation(notation)