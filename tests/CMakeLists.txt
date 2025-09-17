cmake_minimum_required(VERSION 3.16)

# Test executables - organized by functionality
add_executable(test_dice_core test_dice_core.c)
target_link_libraries(test_dice_core dice)

add_executable(test_parser test_parser.c)
target_link_libraries(test_parser dice)

add_executable(test_eval test_eval.c)
target_link_libraries(test_eval dice)

add_executable(test_trace test_trace.c)
target_link_libraries(test_trace dice)

add_executable(test_rng test_rng.c)
target_link_libraries(test_rng dice)

add_executable(test_memory test_memory.c)
target_link_libraries(test_memory dice)

add_executable(test_integration test_integration.c)
target_link_libraries(test_integration dice)

add_executable(test_dice_selection test_dice_selection.c)
target_link_libraries(test_dice_selection dice)

add_executable(test_conditional_selection test_conditional_selection.c)
target_link_libraries(test_conditional_selection dice)

add_executable(test_reroll test_reroll.c)
target_link_libraries(test_reroll dice)

add_executable(test_visitor test_visitor.c)
target_link_libraries(test_visitor dice)

add_executable(test_selection_trace test_selection_trace.c)
target_link_libraries(test_selection_trace dice)

# Legacy test (kept for backwards compatibility)
add_executable(test_dice test_dice.c)
target_link_libraries(test_dice dice)

# Add all tests
add_test(NAME dice_core_tests COMMAND test_dice_core)
add_test(NAME parser_tests COMMAND test_parser)
add_test(NAME eval_tests COMMAND test_eval)
add_test(NAME trace_tests COMMAND test_trace)
add_test(NAME rng_tests COMMAND test_rng)
add_test(NAME memory_tests COMMAND test_memory)
add_test(NAME integration_tests COMMAND test_integration)
add_test(NAME dice_selection_tests COMMAND test_dice_selection)
add_test(NAME conditional_selection_tests COMMAND test_conditional_selection)
add_test(NAME reroll_tests COMMAND test_reroll)
add_test(NAME visitor_tests COMMAND test_visitor)
add_test(NAME selection_trace_tests COMMAND test_selection_trace)
add_test(NAME legacy_dice_tests COMMAND test_dice)