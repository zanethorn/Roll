cmake_minimum_required(VERSION 3.16)
project(Roll VERSION 1.0.0 LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)

# Source files
set(DICE_SOURCES src/dice.c src/parser.c src/eval.c src/trace.c src/rng.c src/memory.c src/custom_dice.c)
set(DICE_HEADERS include/dice.h)

# Options for different build types
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_CONSOLE_APP "Build console application" ON)
option(BUILD_TESTS "Build unit tests" ON)

# Create the dice library
add_library(dice ${DICE_SOURCES} ${DICE_HEADERS})

# Set library properties
set_target_properties(dice PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/dice.h"
)

# Create console application
if(BUILD_CONSOLE_APP)
    add_executable(roll src/cli/roll.c)
    target_link_libraries(roll dice)
endif()

# Enable testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Installation
install(TARGETS dice
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include
)

if(BUILD_CONSOLE_APP)
    install(TARGETS roll
        DESTINATION bin
    )
endif()

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/RollConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_file(cmake/RollConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/RollConfig.cmake"
    @ONLY
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/RollConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/RollConfigVersion.cmake"
    DESTINATION lib/cmake/Roll
)