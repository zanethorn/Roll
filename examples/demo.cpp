#include "roll.h"
#include <iostream>
#include <string>

int main() {
    std::cout << "🎲 Roll - Universal Dice Rolling Library Demo 🎲\n";
    std::cout << "===============================================\n\n";
    
    roll::DiceRoller roller;
    
    // Demonstrate single die
    std::cout << "Single die examples:\n";
    roll::Die d6(6);
    roll::Die d20(20);
    
    std::cout << "Rolling a d6: " << d6.roll() << "\n";
    std::cout << "Rolling a d20: " << d20.roll() << "\n\n";
    
    // Demonstrate multiple dice
    std::cout << "Multiple dice examples:\n";
    auto results_3d6 = roller.roll(3, 6);
    std::cout << "Rolling 3d6: ";
    for (size_t i = 0; i < results_3d6.size(); ++i) {
        if (i > 0) std::cout << " + ";
        std::cout << results_3d6[i];
    }
    std::cout << " = " << roller.rollSum(3, 6) << "\n";
    
    auto results_2d20 = roller.roll(2, 20);
    std::cout << "Rolling 2d20: ";
    for (size_t i = 0; i < results_2d20.size(); ++i) {
        if (i > 0) std::cout << " + ";
        std::cout << results_2d20[i];
    }
    std::cout << " = " << roller.rollSum(2, 20) << "\n\n";
    
    // Demonstrate notation parsing
    std::cout << "Dice notation examples:\n";
    
    std::string notations[] = {"d6", "2d8", "3d10", "4d6", "1d100"};
    
    for (const auto& notation : notations) {
        try {
            auto results = roller.rollFromNotation(notation);
            int sum = roller.rollSumFromNotation(notation);
            
            std::cout << "Rolling " << notation << ": ";
            for (size_t i = 0; i < results.size(); ++i) {
                if (i > 0) std::cout << " + ";
                std::cout << results[i];
            }
            if (results.size() > 1) {
                std::cout << " = " << sum;
            }
            std::cout << "\n";
        } catch (const std::exception& e) {
            std::cout << "Error rolling " << notation << ": " << e.what() << "\n";
        }
    }
    
    std::cout << "\n🎲 Demo complete! Try using the Roll library in your own projects.\n";
    
    return 0;
}