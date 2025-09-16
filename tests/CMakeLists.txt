cmake_minimum_required(VERSION 3.16)

# Simple test framework - just basic functions
add_executable(test_dice test_dice.c)
target_link_libraries(test_dice dice)

# Add the test
add_test(NAME dice_tests COMMAND test_dice)