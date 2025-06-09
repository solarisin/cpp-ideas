
cmake_minimum_required(VERSION 3.25)

# Set project name and version
project(
    CppIdeas
    VERSION 1.0.0
    DESCRIPTION "C++ Ideas and Experiments"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    add_compile_options(-fcolor-diagnostics)
    
    # Debug build options
    add_compile_options($<$<CONFIG:Debug>:-g3 -O0 -fno-omit-frame-pointer>)
    add_compile_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    add_link_options($<$<CONFIG:Debug>:-fsanitize=address,undefined>)
    
    # Release build options
    add_compile_options($<$<CONFIG:Release>:-O3 -DNDEBUG>)
    add_compile_options($<$<CONFIG:Release>:-flto>)
    add_link_options($<$<CONFIG:Release>:-flto>)
endif()

# Find packages
find_package(PkgConfig REQUIRED)

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
endif()

# Find required packages
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Qt6 setup
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Create main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        Qt5::Core
        Qt5::Widgets
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(tests)
add_subdirectory(examples)

# Installation rules
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# CPack configuration for packaging
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_GENERATOR "TGZ;ZIP")
