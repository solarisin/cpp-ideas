cmake_minimum_required(VERSION 3.25)

# Simple example demonstrating fmt usage
add_executable(fmt_example
    fmt_example.cpp
)

target_link_libraries(fmt_example
    PRIVATE
        fmt::fmt
)

# JSON example
add_executable(json_example
    json_example.cpp
)

target_link_libraries(json_example
    PRIVATE
        nlohmann_json::nlohmann_json
        fmt::fmt
)

# Qt example
add_executable(qt_example
    qt_example.cpp
)

target_link_libraries(qt_example
    PRIVATE
        Qt5::Core
        Qt5::Widgets
        fmt::fmt
)

# Set properties for all examples
set_target_properties(fmt_example json_example qt_example
    PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/examples
)
