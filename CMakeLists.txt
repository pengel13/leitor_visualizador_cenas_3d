cmake_minimum_required(VERSION 3.20)

# ============================================================
# Project definition
# ============================================================
project(SceneViewer3D
    VERSION     1.0.0
    DESCRIPTION "3D Scene Viewer using Modern OpenGL (Core Profile)"
    LANGUAGES   CXX
)

# Require C++17 for std::filesystem and structured bindings
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

# ============================================================
# Dependencies — resolved via vcpkg toolchain.
#
# Install vcpkg and run:
#   vcpkg install glad glfw3 glm assimp
#
# Then configure CMake with:
#   cmake -S . -B build \
#     -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake
#
# All find_package calls below use CONFIG mode which vcpkg supports.
# ============================================================

# GLAD — OpenGL function loader (replaces <GL/gl.h>)
find_package(glad    CONFIG REQUIRED)

# GLFW — cross-platform window + input
find_package(glfw3   CONFIG REQUIRED)

# GLM — header-only math library (vectors, matrices, quaternions)
find_package(glm     CONFIG REQUIRED)

# Assimp — 3D model import library (.obj, .ply, .fbx, .gltf, …)
find_package(assimp  CONFIG REQUIRED)

# ============================================================
# Source files
# ============================================================
set(SOURCES
    src/main.cpp
    src/Shader.cpp
    src/Camera.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/Object3D.cpp
    src/Scene.cpp
    src/Renderer.cpp
    src/Primitives.cpp
)

# ============================================================
# Executable target
# ============================================================
add_executable(SceneViewer3D ${SOURCES})

# Include our own src/ directory so headers can be included without paths
target_include_directories(SceneViewer3D PRIVATE src)

# ============================================================
# Link libraries
# ============================================================
target_link_libraries(SceneViewer3D PRIVATE
    glad::glad          # OpenGL function pointers
    glfw                # windowing + input
    glm::glm            # math (header-only, but CMake target handles includes)
    assimp::assimp      # 3D model loading
)

# On some Linux systems you also need to link against OpenGL explicitly
if(UNIX AND NOT APPLE)
    target_link_libraries(SceneViewer3D PRIVATE GL)
endif()

# ============================================================
# Compiler warnings
# ============================================================
if(MSVC)
    target_compile_options(SceneViewer3D PRIVATE
        /W4         # Warning level 4
        /WX-        # Do not treat warnings as errors (libraries may emit them)
        /wd4251     # Suppress DLL-interface warnings from assimp
    )
else()
    target_compile_options(SceneViewer3D PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter    # GLM headers sometimes trigger this
    )
endif()

# ============================================================
# Post-build: copy runtime assets to build output directory
#
# The executable looks for shaders/ and assets/ in its working
# directory.  These commands copy them there after each build.
# ============================================================

# Copy shaders/
add_custom_command(TARGET SceneViewer3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:SceneViewer3D>/shaders"
    COMMENT "Copying shaders to output directory"
)

# Copy assets/
add_custom_command(TARGET SceneViewer3D POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:SceneViewer3D>/assets"
    COMMENT "Copying assets to output directory"
)

# ============================================================
# Build info
# ============================================================
message(STATUS "CMake version:   ${CMAKE_VERSION}")
message(STATUS "Build type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard:    ${CMAKE_CXX_STANDARD}")
message(STATUS "Target:          SceneViewer3D ${PROJECT_VERSION}")
