# Add test executable
add_executable(basic_test basic_test.cpp)

# Link against the roll library
target_link_libraries(basic_test roll)

# Register as a test
add_test(NAME basic_test COMMAND basic_test)