cmake_minimum_required(VERSION 3.30)

# Find testing framework
find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    # If Catch2 is not found, download it using FetchContent
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0
    )
    FetchContent_MakeAvailable(Catch2)
    list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
endif()

find_package(loguru CONFIG REQUIRED)

# Create test executable
add_executable(tests
    test_main.cpp
    test_json.cpp
    test_formatting.cpp
    test_stuff.cpp
    test_python_processor.cpp
)

# Link libraries
target_link_libraries(tests
    PRIVATE
        Catch2::Catch2WithMain
        fmt::fmt
        nlohmann_json::nlohmann_json
        Qt::Core
        loguru::loguru
        python_processor_lib
)

# Include test discovery
include(CTest)
include(Catch)
catch_discover_tests(tests WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Set properties
set_target_properties(tests PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ENABLE_EXPORTS ON
)
