cmake_minimum_required(VERSION 3.30)

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

set(PROJECT_OPTS ${PROJECT_NAME}_opts)
add_library(${PROJECT_OPTS} INTERFACE)

# Add compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Warnings
    target_compile_options(${PROJECT_OPTS} 
        INTERFACE
            -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Werror
    )

    # Misc
    target_compile_options(${PROJECT_OPTS}
        INTERFACE 
            -fcolor-diagnostics)
    
    # Debug build options
    target_compile_options(${PROJECT_OPTS}
        INTERFACE
            $<$<CONFIG:Debug>:-g3>
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:Debug>:-fno-omit-frame-pointer>
    )

    # Sanitizer options
    target_compile_options(${PROJECT_OPTS}
        INTERFACE
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
    target_link_options(${PROJECT_OPTS}
        INTERFACE
            $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
    
    # Release build options
    target_compile_options(${PROJECT_OPTS}
        INTERFACE
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Release>:-DNDEBUG>
    )

    # lto
    target_compile_options(${PROJECT_OPTS}
        INTERFACE
            $<$<CONFIG:Release>:-flto>)
    target_link_options(${PROJECT_OPTS}
        INTERFACE
            $<$<CONFIG:Release>:-flto>)
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
find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
find_package(Boost REQUIRED COMPONENTS python312)
find_package(loguru CONFIG REQUIRED)

# Qt6 setup
find_package(Qt6 COMPONENTS Core Widgets REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${Python3_INCLUDE_DIRS}
)


# Create main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/python_processor.cpp
)

#target-specific compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Werror)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    INTERFACE ${PROJECT_OPTS}
    PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        Qt::Core
        Qt::Widgets
        Python3::Python
        Boost::python312
        loguru::loguru
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    ENABLE_EXPORTS ON # Ensure Python symbols are exported for dynamic loading of extension modules
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(tests)

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
