# Create the roll library
add_library(roll STATIC roll.cpp)

# Set properties
set_target_properties(roll PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Link dependencies (none currently needed)
# target_link_libraries(roll)

# Make headers available to the library
target_include_directories(roll PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)