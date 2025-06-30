cmake_minimum_required(VERSION 3.28)

# Set project name and version
project(
    cppideas
    VERSION 1.0.0
    DESCRIPTION "C++ Ideas and Experiments"
    LANGUAGES CXX
)

# Set C++ standard
if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
endif()
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
find_package(OpenGL REQUIRED)

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    if(DEFINED VCPKG_INSTALLED_DIR AND DEFINED VCPKG_TARGET_TRIPLET)
        set(CPPIDEAS_DEPS_DIR "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
    else()
        message(FATAL_ERROR "VCPKG_INSTALLED_DIR or VCPKG_TARGET_TRIPLET is not defined. Please set them in your environment.")
    endif()
else()
    message(WARNING "VCPKG_ROOT environment variable is not set. Using system packages.")
endif()

# Find required packages
find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(loguru CONFIG REQUIRED)

# Create main executable
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/tictactoe.cpp
    src/simple_nn.cpp
    src/nn_tictactoe_player.cpp
    src/random_utils.cpp
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    ENABLE_EXPORTS ON # Ensure Python symbols are exported for dynamic loading of extension modules
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

#target-specific compile options
target_compile_options(${PROJECT_NAME} PRIVATE -Werror)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    INTERFACE ${PROJECT_OPTS}
    PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        loguru::loguru
)

# Create Tic Tac Toe neural network executable
add_executable(tictactoe_nn
    src/main.cpp
    src/tictactoe.cpp
    src/simple_nn.cpp
    src/nn_tictactoe_player.cpp
    src/random_utils.cpp
)

#target-specific compile options
target_compile_options(tictactoe_nn PRIVATE -Werror)

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(tictactoe_nn
    INTERFACE ${PROJECT_OPTS}
    PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        loguru::loguru
)

# ImGui GUI executable
add_executable(tictactoe_nn_imgui
    src/main_imgui.cpp
    src/tictactoe.cpp
    src/simple_nn.cpp
    src/nn_tictactoe_player.cpp
    src/random_utils.cpp
    src/imgui_impl_glfw.cpp
    src/imgui_impl_opengl3.cpp
)

set_target_properties(tictactoe_nn_imgui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

target_compile_options(tictactoe_nn_imgui PRIVATE -Werror)

target_include_directories(tictactoe_nn_imgui PRIVATE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/imgui
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/include/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_directories(tictactoe_nn_imgui PRIVATE
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib
    ${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/debug/lib
)

target_link_libraries(tictactoe_nn_imgui
    INTERFACE ${PROJECT_OPTS}
    PRIVATE
        fmt::fmt
        nlohmann_json::nlohmann_json
        loguru::loguru
        imgui
        glfw
        OpenGL::GL
)

# Enable testing
enable_testing()

# Add subdirectories
add_subdirectory(tests)

# Installation rules
install(TARGETS ${PROJECT_NAME} tictactoe_nn tictactoe_nn_imgui
    RUNTIME DESTINATION bin
)

# CPack configuration for packaging
include(CPack)
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
set(CPACK_GENERATOR "TGZ;ZIP")
