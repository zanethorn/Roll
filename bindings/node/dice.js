/**
 * Node.js bindings for the Roll dice library
 * 
 * This library provides JavaScript bindings for the universal dice rolling library
 * using a hybrid approach with subprocess calls to the native binary for reliability.
 */

const { execSync, spawn } = require('child_process');
const path = require('path');

class DiceError extends Error {
    constructor(message) {
        super(message);
        this.name = 'DiceError';
    }
}

class Dice {
    constructor() {
        // Path to the roll binary (adjust as needed for installation)
        this.rollPath = path.join(__dirname, '..', '..', 'build', 'roll');
        
        // Check if binary exists
        try {
            execSync(`"${this.rollPath}" --version`, { encoding: 'utf8' });
        } catch (error) {
            // Fallback to system-installed version
            try {
                execSync('roll --version', { encoding: 'utf8' });
                this.rollPath = 'roll';
            } catch (fallbackError) {
                throw new DiceError('Roll binary not found. Please build the C library first.');
            }
        }
        
        // Set seed if provided
        this.seed = null;
    }

    /**
     * Initialize with a seed
     * @param {number|null} seed - Random seed (null for time-based)
     */
    init(seed = null) {
        this.seed = seed;
    }

    /**
     * Get library version
     * @returns {string} Version string
     */
    version() {
        try {
            const output = execSync(`"${this.rollPath}" --version`, { encoding: 'utf8' });
            const match = output.match(/Roll (\d+\.\d+\.\d+)/);
            return match ? match[1] : 'unknown';
        } catch (error) {
            throw new DiceError('Failed to get version');
        }
    }

    /**
     * Roll dice using notation
     * @param {string} notation - Dice notation like "3d6", "1d20+5"
     * @param {number} count - Number of times to roll (default: 1)
     * @returns {number|number[]} Single result or array of results
     */
    rollNotation(notation, count = 1) {
        if (!notation || typeof notation !== 'string') {
            throw new DiceError('Invalid dice notation');
        }

        try {
            const args = [];
            
            if (this.seed !== null) {
                args.push('--seed', this.seed.toString());
            }
            
            if (count > 1) {
                args.push('--count', count.toString());
            }
            
            args.push(notation);
            
            const output = execSync(`"${this.rollPath}" ${args.join(' ')}`, { encoding: 'utf8' });
            const lines = output.trim().split('\n');
            
            if (count === 1) {
                const result = parseInt(lines[0]);
                if (isNaN(result)) {
                    throw new DiceError('Invalid result from dice roll');
                }
                return result;
            } else {
                const results = [];
                for (const line of lines) {
                    const match = line.match(/Roll \d+: (\d+)/);
                    if (match) {
                        results.push(parseInt(match[1]));
                    }
                }
                return results;
            }
        } catch (error) {
            if (error.status) {
                throw new DiceError(`Dice roll failed: ${error.message}`);
            }
            throw error;
        }
    }

    /**
     * Roll a single die
     * @param {number} sides - Number of sides
     * @returns {number} Result between 1 and sides
     */
    roll(sides) {
        return this.rollNotation(`1d${sides}`);
    }

    /**
     * Roll multiple dice
     * @param {number} count - Number of dice
     * @param {number} sides - Number of sides on each die
     * @returns {number} Sum of all rolls
     */
    rollMultiple(count, sides) {
        return this.rollNotation(`${count}d${sides}`);
    }

    /**
     * Roll multiple dice and get individual results
     * @param {number} count - Number of dice
     * @param {number} sides - Number of sides on each die  
     * @returns {{sum: number, individual: number[]}} Object with sum and individual results
     */
    rollIndividual(count, sides) {
        // For individual results, we need to roll them one by one
        const individual = [];
        let sum = 0;
        
        for (let i = 0; i < count; i++) {
            const result = this.roll(sides);
            individual.push(result);
            sum += result;
        }
        
        return { sum, individual };
    }
}

// Create a default instance
const dice = new Dice();

// Export both class and convenience functions
module.exports = {
    Dice,
    DiceError,
    
    // Convenience functions using default instance
    init: (seed) => dice.init(seed),
    version: () => dice.version(),
    roll: (sides) => dice.roll(sides),
    rollMultiple: (count, sides) => dice.rollMultiple(count, sides),
    rollIndividual: (count, sides) => dice.rollIndividual(count, sides),
    rollNotation: (notation, count) => dice.rollNotation(notation, count)
};