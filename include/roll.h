#pragma once

#include <random>
#include <vector>
#include <string>

namespace roll {

/**
 * @brief A class representing a single die
 */
class Die {
public:
    /**
     * @brief Construct a new Die object
     * @param sides Number of sides on the die (default: 6)
     */
    explicit Die(int sides = 6);
    
    /**
     * @brief Roll the die once
     * @return Random value between 1 and sides (inclusive)
     */
    int roll();
    
    /**
     * @brief Get the number of sides on this die
     * @return Number of sides
     */
    int getSides() const;

private:
    int sides_;
    std::mt19937 generator_;
    std::uniform_int_distribution<int> distribution_;
};

/**
 * @brief A class for rolling multiple dice
 */
class DiceRoller {
public:
    /**
     * @brief Construct a new Dice Roller object
     */
    DiceRoller();
    
    /**
     * @brief Roll multiple dice
     * @param count Number of dice to roll
     * @param sides Number of sides per die (default: 6)
     * @return Vector of roll results
     */
    std::vector<int> roll(int count, int sides = 6);
    
    /**
     * @brief Roll dice and return the sum
     * @param count Number of dice to roll
     * @param sides Number of sides per die (default: 6)
     * @return Sum of all rolls
     */
    int rollSum(int count, int sides = 6);
    
    /**
     * @brief Parse and roll dice from string notation (e.g., "3d6", "2d20")
     * @param notation Dice notation string
     * @return Vector of roll results
     */
    std::vector<int> rollFromNotation(const std::string& notation);
    
    /**
     * @brief Parse and roll dice from string notation, return sum
     * @param notation Dice notation string
     * @return Sum of all rolls
     */
    int rollSumFromNotation(const std::string& notation);

private:
    std::mt19937 generator_;
    
    /**
     * @brief Parse dice notation string (e.g., "3d6" -> count=3, sides=6)
     * @param notation Dice notation string
     * @param count Output parameter for dice count
     * @param sides Output parameter for dice sides
     * @return true if parsing was successful, false otherwise
     */
    bool parseNotation(const std::string& notation, int& count, int& sides);
};

} // namespace roll