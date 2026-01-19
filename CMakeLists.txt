cmake_minimum_required(VERSION 3.29)
project(star_engine VERSION 0.1.0 DESCRIPTION "New gen StarFire game engine" LANGUAGES CXX)

# Define source directories
set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(TEST_DIR "${PROJECT_SOURCE_DIR}/tests")
set(EDITOR_DIR "${PROJECT_SOURCE_DIR}/editor")
set(SAMPLES_DIR "${PROJECT_SOURCE_DIR}/samples")

# Scripts for external packages
message(STATUS "Fetching packages...")
include(scripts/bgfx.cmake)
include(scripts/sdl3.cmake)
include(scripts/spdlog.cmake)
include(scripts/flecs.cmake)
include(scripts/freetype.cmake)
include(scripts/imgui.cmake)

# Project Files
file(GLOB_RECURSE ENGINE_SRC_FILES
        "${SRC_DIR}/**.cpp"
        "${SRC_DIR}/**.hpp"
        "${SRC_DIR}/**.h"
)

file(GLOB_RECURSE INCLUDE_FILES
        "${INCLUDE_DIR}/**.hpp"
        "${INCLUDE_DIR}/**.h"
)

file(GLOB_RECURSE EDITOR_FILES
        "${EDITOR_DIR}/**.cpp"
        "${EDITOR_DIR}/**.hpp"
        "${EDITOR_DIR}/**.h"
)

file(GLOB_RECURSE TEST_FILES
        "${TEST_DIR}/**.cpp"
        "${TEST_DIR}/**.hpp"
        "${TEST_DIR}/**.h"
)

file(GLOB_RECURSE SAMPLES_FILES
        "${SAMPLES_DIR}/**.cpp"
        "${SAMPLES_DIR}/**.hpp"
        "${SAMPLES_DIR}/**.h"
)

add_library(${PROJECT_NAME} ${ENGINE_SRC_FILES} ${INCLUDE_FILES})

# Set compiler options
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 26)

target_compile_definitions(${PROJECT_NAME} PUBLIC STAR_USE_SDL)

# Source grouping
source_group(TREE "${SRC_DIR}" PREFIX "src" FILES ${ENGINE_SRC_FILES})
source_group(TREE "${INCLUDE_DIR}" PREFIX "include" FILES ${INCLUDE_FILES})

# Precompiled Headers
target_precompile_headers(${PROJECT_NAME} PRIVATE "${INCLUDE_DIR}/star/core/common.hpp")

# Shader Compilation
message(STATUS "Compiling shaders...")
file(GLOB VERTEX_SHADER_FILES "assets/shaders/v_*.sc")
file(GLOB FRAGMENT_SHADER_FILES "assets/shaders/f_*.sc")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/include/generated/shaders")

bgfx_compile_shaders(
        TYPE VERTEX
        SHADERS ${VERTEX_SHADER_FILES}
        VARYING_DEF "${PROJECT_SOURCE_DIR}/assets/shaders/varying.def.sc"
        OUTPUT_DIR "${CMAKE_BINARY_DIR}/include/generated/shaders"
        AS_HEADERS
)

bgfx_compile_shaders(
        TYPE FRAGMENT
        SHADERS ${FRAGMENT_SHADER_FILES}
        VARYING_DEF "${PROJECT_SOURCE_DIR}/assets/shaders/varying.def.sc"
        OUTPUT_DIR "${CMAKE_BINARY_DIR}/include/generated/shaders"
        AS_HEADERS
)

add_library(shaders INTERFACE)
target_sources(shaders INTERFACE ${VERTEX_SHADER_FILES} ${FRAGMENT_SHADER_FILES})
target_include_directories(shaders INTERFACE "${CMAKE_BINARY_DIR}/include/generated/shaders")

add_dependencies(${PROJECT_NAME} shaders)

# Configure include directories
target_include_directories(${PROJECT_NAME} PUBLIC
        "${SRC_DIR}"
        "${INCLUDE_DIR}"
        "${BGFX_INCLUDE_DIR}"
        "${BIMG_INCLUDE_DIR}"
        "${BX_INCLUDE_DIR}"
        "${SDL3_INCLUDE_DIR}"
        "${spdlog_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/misc/freetype"
        "${freetype_SOURCE_DIR}/include"
        "${flecs_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/include/generated/shaders"
)

# Link dependencies for the library
target_link_libraries(${PROJECT_NAME} PUBLIC
        bgfx
        bimg
        bx
        SDL3::SDL3
        spdlog::spdlog
        shaders
        imgui
        freetype
        flecs::flecs_static
        ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_BINARY_DIR}/assets"
)

# C++ Hot reloading
if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif ()

if (EXISTS "${EDITOR_DIR}/CMakeLists.txt")
    add_subdirectory(${EDITOR_DIR})
endif ()

if (EXISTS "${SAMPLES_DIR}/CMakeLists.txt")
    add_subdirectory(${SAMPLES_DIR})
endif ()

if (EXISTS "${TEST_DIR}/CMakeLists.txt")
    add_subdirectory(${TEST_DIR})
endif ()