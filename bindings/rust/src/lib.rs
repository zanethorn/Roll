//! Rust bindings for the Roll dice library
//! 
//! This library provides safe Rust bindings for the universal dice rolling library.

use libc::{c_char, c_int, c_uint};
use std::ffi::{CStr, CString};
use std::fmt;

// External C functions
extern "C" {
    fn dice_init(seed: c_uint);
    fn dice_roll(sides: c_int) -> c_int;
    fn dice_roll_multiple(count: c_int, sides: c_int) -> c_int;
    fn dice_roll_individual(count: c_int, sides: c_int, results: *mut c_int) -> c_int;
    fn dice_roll_notation(dice_notation: *const c_char) -> c_int;
    fn dice_version() -> *const c_char;
}

/// Error type for dice operations
#[derive(Debug, Clone, PartialEq)]
pub enum DiceError {
    InvalidSides(i32),
    InvalidCount(i32),
    InvalidNotation(String),
    NullPointer,
}

impl fmt::Display for DiceError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            DiceError::InvalidSides(sides) => write!(f, "Invalid number of sides: {}", sides),
            DiceError::InvalidCount(count) => write!(f, "Invalid count: {}", count),
            DiceError::InvalidNotation(notation) => write!(f, "Invalid dice notation: {}", notation),
            DiceError::NullPointer => write!(f, "Null pointer error"),
        }
    }
}

impl std::error::Error for DiceError {}

/// Result type for dice operations
pub type DiceResult<T> = Result<T, DiceError>;

/// Main dice library interface
pub struct Dice;

impl Dice {
    /// Initialize the dice library with a seed
    /// 
    /// # Arguments
    /// 
    /// * `seed` - Random seed (None for time-based seed)
    pub fn init(seed: Option<u32>) {
        unsafe {
            dice_init(seed.unwrap_or(0));
        }
    }
    
    /// Get the library version
    pub fn version() -> String {
        unsafe {
            let version_ptr = dice_version();
            if version_ptr.is_null() {
                return "unknown".to_string();
            }
            CStr::from_ptr(version_ptr)
                .to_string_lossy()
                .to_string()
        }
    }
    
    /// Roll a single die
    /// 
    /// # Arguments
    /// 
    /// * `sides` - Number of sides on the die
    /// 
    /// # Returns
    /// 
    /// Random value between 1 and sides (inclusive)
    pub fn roll(sides: i32) -> DiceResult<i32> {
        if sides <= 0 {
            return Err(DiceError::InvalidSides(sides));
        }
        
        unsafe {
            let result = dice_roll(sides);
            if result == -1 {
                Err(DiceError::InvalidSides(sides))
            } else {
                Ok(result)
            }
        }
    }
    
    /// Roll multiple dice and return the sum
    /// 
    /// # Arguments
    /// 
    /// * `count` - Number of dice to roll
    /// * `sides` - Number of sides on each die
    /// 
    /// # Returns
    /// 
    /// Sum of all dice rolls
    pub fn roll_multiple(count: i32, sides: i32) -> DiceResult<i32> {
        if count <= 0 {
            return Err(DiceError::InvalidCount(count));
        }
        if sides <= 0 {
            return Err(DiceError::InvalidSides(sides));
        }
        
        unsafe {
            let result = dice_roll_multiple(count, sides);
            if result == -1 {
                if count <= 0 {
                    Err(DiceError::InvalidCount(count))
                } else {
                    Err(DiceError::InvalidSides(sides))
                }
            } else {
                Ok(result)
            }
        }
    }
    
    /// Roll multiple dice and return individual results
    /// 
    /// # Arguments
    /// 
    /// * `count` - Number of dice to roll  
    /// * `sides` - Number of sides on each die
    /// 
    /// # Returns
    /// 
    /// Tuple of (sum, vector of individual results)
    pub fn roll_individual(count: i32, sides: i32) -> DiceResult<(i32, Vec<i32>)> {
        if count <= 0 {
            return Err(DiceError::InvalidCount(count));
        }
        if sides <= 0 {
            return Err(DiceError::InvalidSides(sides));
        }
        
        let mut results = vec![0i32; count as usize];
        
        unsafe {
            let sum = dice_roll_individual(count, sides, results.as_mut_ptr());
            if sum == -1 {
                if count <= 0 {
                    Err(DiceError::InvalidCount(count))
                } else {
                    Err(DiceError::InvalidSides(sides))
                }
            } else {
                Ok((sum, results))
            }
        }
    }
    
    /// Roll dice using RPG notation
    /// 
    /// # Arguments
    /// 
    /// * `notation` - Dice notation like "3d6", "1d20+5", "2d8-1"
    /// 
    /// # Returns
    /// 
    /// Result of the dice roll
    pub fn roll_notation(notation: &str) -> DiceResult<i32> {
        let c_notation = CString::new(notation)
            .map_err(|_| DiceError::InvalidNotation(notation.to_string()))?;
        
        unsafe {
            let result = dice_roll_notation(c_notation.as_ptr());
            if result == -1 {
                Err(DiceError::InvalidNotation(notation.to_string()))
            } else {
                Ok(result)
            }
        }
    }
}

/// Convenience functions
pub fn init(seed: Option<u32>) {
    Dice::init(seed);
}

pub fn version() -> String {
    Dice::version()
}

pub fn roll(sides: i32) -> DiceResult<i32> {
    Dice::roll(sides)
}

pub fn roll_multiple(count: i32, sides: i32) -> DiceResult<i32> {
    Dice::roll_multiple(count, sides)
}

pub fn roll_individual(count: i32, sides: i32) -> DiceResult<(i32, Vec<i32>)> {
    Dice::roll_individual(count, sides)
}

pub fn roll_notation(notation: &str) -> DiceResult<i32> {
    Dice::roll_notation(notation)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_version() {
        let version = Dice::version();
        assert!(!version.is_empty());
        println!("Library version: {}", version);
    }

    #[test]
    fn test_initialization() {
        Dice::init(Some(12345));
        Dice::init(None);
    }

    #[test]
    fn test_single_roll() {
        Dice::init(Some(12345));
        
        // Test valid rolls
        let result = Dice::roll(6).unwrap();
        assert!(result >= 1 && result <= 6);
        
        let result = Dice::roll(20).unwrap();
        assert!(result >= 1 && result <= 20);
        
        // Test invalid rolls
        assert!(Dice::roll(0).is_err());
        assert!(Dice::roll(-5).is_err());
    }

    #[test]
    fn test_multiple_rolls() {
        Dice::init(Some(12345));
        
        // Test valid rolls
        let result = Dice::roll_multiple(3, 6).unwrap();
        assert!(result >= 3 && result <= 18);
        
        let result = Dice::roll_multiple(1, 20).unwrap();
        assert!(result >= 1 && result <= 20);
        
        // Test invalid rolls
        assert!(Dice::roll_multiple(0, 6).is_err());
        assert!(Dice::roll_multiple(3, 0).is_err());
    }

    #[test]
    fn test_individual_rolls() {
        Dice::init(Some(12345));
        
        // Test valid rolls
        let (sum, individual) = Dice::roll_individual(3, 6).unwrap();
        assert!(sum >= 3 && sum <= 18);
        assert_eq!(individual.len(), 3);
        assert!(individual.iter().all(|&roll| roll >= 1 && roll <= 6));
        assert_eq!(sum, individual.iter().sum());
        
        // Test single die
        let (sum, individual) = Dice::roll_individual(1, 20).unwrap();
        assert!(sum >= 1 && sum <= 20);
        assert_eq!(individual.len(), 1);
        assert_eq!(sum, individual[0]);
    }

    #[test]
    fn test_notation_rolls() {
        Dice::init(Some(12345));
        
        // Test basic notation
        let result = Dice::roll_notation("1d6").unwrap();
        assert!(result >= 1 && result <= 6);
        
        let result = Dice::roll_notation("3d6").unwrap();
        assert!(result >= 3 && result <= 18);
        
        // Test with modifiers
        let result = Dice::roll_notation("1d6+5").unwrap();
        assert!(result >= 6 && result <= 11);
        
        let result = Dice::roll_notation("1d6-1").unwrap();
        assert!(result >= 0 && result <= 5);
        
        // Test uppercase D
        let result = Dice::roll_notation("1D6").unwrap();
        assert!(result >= 1 && result <= 6);
        
        // Test invalid notation
        assert!(Dice::roll_notation("invalid").is_err());
    }
}