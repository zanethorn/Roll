#include "roll.h"
#include <algorithm>
#include <stdexcept>
#include <regex>
#include <numeric>

namespace roll {

// Die implementation
Die::Die(int sides) 
    : sides_(sides)
    , generator_(std::random_device{}())
    , distribution_(1, sides)
{
    if (sides <= 0) {
        throw std::invalid_argument("Die must have at least 1 side");
    }
}

int Die::roll() {
    return distribution_(generator_);
}

int Die::getSides() const {
    return sides_;
}

// DiceRoller implementation
DiceRoller::DiceRoller() 
    : generator_(std::random_device{}()) 
{
}

std::vector<int> DiceRoller::roll(int count, int sides) {
    if (count <= 0) {
        throw std::invalid_argument("Must roll at least 1 die");
    }
    if (sides <= 0) {
        throw std::invalid_argument("Die must have at least 1 side");
    }
    
    std::vector<int> results;
    results.reserve(count);
    
    std::uniform_int_distribution<int> distribution(1, sides);
    
    for (int i = 0; i < count; ++i) {
        results.push_back(distribution(generator_));
    }
    
    return results;
}

int DiceRoller::rollSum(int count, int sides) {
    auto results = roll(count, sides);
    return std::accumulate(results.begin(), results.end(), 0);
}

std::vector<int> DiceRoller::rollFromNotation(const std::string& notation) {
    int count, sides;
    if (!parseNotation(notation, count, sides)) {
        throw std::invalid_argument("Invalid dice notation: " + notation);
    }
    return roll(count, sides);
}

int DiceRoller::rollSumFromNotation(const std::string& notation) {
    auto results = rollFromNotation(notation);
    return std::accumulate(results.begin(), results.end(), 0);
}

bool DiceRoller::parseNotation(const std::string& notation, int& count, int& sides) {
    // Regular expression to match patterns like "3d6", "2d20", "d6" (assumes 1d6)
    std::regex dice_regex(R"(^(\d*)d(\d+)$)", std::regex_constants::icase);
    std::smatch match;
    
    if (!std::regex_match(notation, match, dice_regex)) {
        return false;
    }
    
    // Parse count (default to 1 if empty)
    std::string count_str = match[1].str();
    if (count_str.empty()) {
        count = 1;
    } else {
        try {
            count = std::stoi(count_str);
        } catch (const std::exception&) {
            return false;
        }
    }
    
    // Parse sides
    try {
        sides = std::stoi(match[2].str());
    } catch (const std::exception&) {
        return false;
    }
    
    return count > 0 && sides > 0;
}

} // namespace roll