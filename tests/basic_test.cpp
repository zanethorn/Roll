#include "roll.h"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Running basic tests for Roll library...\n";
    
    // Test Die class
    {
        roll::Die d6(6);
        assert(d6.getSides() == 6);
        
        // Roll a few times and check range
        for (int i = 0; i < 10; ++i) {
            int result = d6.roll();
            assert(result >= 1 && result <= 6);
        }
        std::cout << "✓ Die class tests passed\n";
    }
    
    // Test DiceRoller class
    {
        roll::DiceRoller roller;
        
        // Test basic roll
        auto results = roller.roll(3, 6);
        assert(results.size() == 3);
        for (int result : results) {
            assert(result >= 1 && result <= 6);
        }
        std::cout << "✓ DiceRoller basic roll tests passed\n";
        
        // Test roll sum
        int sum = roller.rollSum(2, 6);
        assert(sum >= 2 && sum <= 12);
        std::cout << "✓ DiceRoller sum tests passed\n";
        
        // Test notation parsing
        auto notation_results = roller.rollFromNotation("3d6");
        assert(notation_results.size() == 3);
        for (int result : notation_results) {
            assert(result >= 1 && result <= 6);
        }
        std::cout << "✓ Dice notation tests passed\n";
        
        // Test single die notation
        auto single_result = roller.rollFromNotation("d20");
        assert(single_result.size() == 1);
        assert(single_result[0] >= 1 && single_result[0] <= 20);
        std::cout << "✓ Single die notation tests passed\n";
    }
    
    std::cout << "\n🎲 All tests passed! Roll library is working correctly.\n";
    return 0;
}