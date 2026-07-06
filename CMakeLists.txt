cmake_minimum_required(VERSION 3.29)
project(star_engine VERSION 0.1.0 DESCRIPTION "New gen StarFire game engine" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif ()

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
include(scripts/imguizmo.cmake)
include(scripts/nlohmann.cmake)
include(scripts/jolt.cmake)
include(scripts/stb.cmake)
include(scripts/cgltf.cmake)

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

file(GLOB_RECURSE STAR_CORE_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/core/*.cpp" "${SRC_DIR}/core/*.hpp" "${SRC_DIR}/core/*.h"
        "${INCLUDE_DIR}/star/core/*.hpp" "${INCLUDE_DIR}/star/core/*.h"
)
file(GLOB_RECURSE STAR_MATH_HEADERS CONFIGURE_DEPENDS
        "${INCLUDE_DIR}/star/math/*.hpp" "${INCLUDE_DIR}/star/math/*.h"
)
file(GLOB_RECURSE STAR_PLATFORM_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/platform/*.cpp" "${SRC_DIR}/platform/*.hpp" "${SRC_DIR}/platform/*.h"
        "${INCLUDE_DIR}/star/platform/*.hpp" "${INCLUDE_DIR}/star/platform/*.h"
)
file(GLOB_RECURSE STAR_GRAPHICS_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/graphics/*.cpp" "${SRC_DIR}/graphics/*.hpp" "${SRC_DIR}/graphics/*.h"
        "${INCLUDE_DIR}/star/graphics/*.hpp" "${INCLUDE_DIR}/star/graphics/*.h"
)
file(GLOB_RECURSE STAR_RESOURCES_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/resources/*.cpp" "${SRC_DIR}/resources/*.hpp" "${SRC_DIR}/resources/*.h"
        "${INCLUDE_DIR}/star/resources/*.hpp" "${INCLUDE_DIR}/star/resources/*.h"
)
file(GLOB_RECURSE STAR_ECS_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/ecs/*.cpp" "${SRC_DIR}/ecs/*.hpp" "${SRC_DIR}/ecs/*.h"
        "${INCLUDE_DIR}/star/ecs/*.hpp" "${INCLUDE_DIR}/star/ecs/*.h"
)
file(GLOB_RECURSE STAR_RENDERING_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/rendering/*.cpp" "${SRC_DIR}/rendering/*.hpp" "${SRC_DIR}/rendering/*.h"
        "${INCLUDE_DIR}/star/rendering/*.hpp" "${INCLUDE_DIR}/star/rendering/*.h"
)
file(GLOB_RECURSE STAR_SCENE_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/scene/*.cpp" "${SRC_DIR}/scene/*.hpp" "${SRC_DIR}/scene/*.h"
        "${INCLUDE_DIR}/star/scene/*.hpp" "${INCLUDE_DIR}/star/scene/*.h"
)
file(GLOB_RECURSE STAR_APPLICATION_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/application/*.cpp" "${SRC_DIR}/application/*.hpp" "${SRC_DIR}/application/*.h"
        "${INCLUDE_DIR}/star/application/*.hpp" "${INCLUDE_DIR}/star/application/*.h"
)
file(GLOB_RECURSE STAR_IMGUI_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/imgui/*.cpp" "${SRC_DIR}/imgui/*.hpp" "${SRC_DIR}/imgui/*.h"
        "${INCLUDE_DIR}/star/imgui/*.hpp" "${INCLUDE_DIR}/star/imgui/*.h"
)
file(GLOB_RECURSE STAR_UTILS_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/utils/*.cpp" "${SRC_DIR}/utils/*.hpp" "${SRC_DIR}/utils/*.h"
        "${INCLUDE_DIR}/star/utils/*.hpp" "${INCLUDE_DIR}/star/utils/*.h"
)

file(GLOB_RECURSE STAR_PHYSICS_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/physics/*.cpp" "${SRC_DIR}/physics/*.hpp" "${SRC_DIR}/physics/*.h"
        "${INCLUDE_DIR}/star/physics/*.hpp" "${INCLUDE_DIR}/star/physics/*.h"
)

file(GLOB_RECURSE STAR_PROJECT_SRC CONFIGURE_DEPENDS
        "${SRC_DIR}/project/*.cpp" "${SRC_DIR}/project/*.hpp" "${SRC_DIR}/project/*.h"
        "${INCLUDE_DIR}/star/project/*.hpp" "${INCLUDE_DIR}/star/project/*.h"
)

function(star_add_module name)
    cmake_parse_arguments(SAM "" "" "SOURCES;DEPS;INCLUDES" ${ARGN})
    add_library(${name} OBJECT ${SAM_SOURCES})
    set_target_properties(${name} PROPERTIES
            CXX_STANDARD 23
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
            POSITION_INDEPENDENT_CODE ON
            FOLDER "star_engine"
    )
    target_include_directories(${name} PUBLIC
            "${SRC_DIR}"
            "${INCLUDE_DIR}"
            "${CMAKE_BINARY_DIR}/include/generated/shaders"
    )
    if (SAM_INCLUDES)
        target_include_directories(${name} PUBLIC ${SAM_INCLUDES})
    endif ()
    target_compile_definitions(${name} PUBLIC
            STAR_USE_SDL
            STAR_GRAPHICS_API_BGFX
            IMGUI_ENABLE_FREETYPE
    )
    if (WIN32)
        target_compile_definitions(${name} PUBLIC
                WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS NOGDI NOSERVICE
        )
    endif ()
    if (MSVC)
        target_compile_options(${name} PUBLIC /utf-8 /Zc:__cplusplus /Zc:preprocessor /permissive-)
    endif ()
    if (SAM_DEPS)
        target_link_libraries(${name} PUBLIC ${SAM_DEPS})
    endif ()
endfunction()

add_library(star_math INTERFACE)
target_sources(star_math INTERFACE ${STAR_MATH_HEADERS})
target_include_directories(star_math INTERFACE "${INCLUDE_DIR}")

star_add_module(star_core
        SOURCES ${STAR_CORE_SRC} ${STAR_UTILS_SRC}
        INCLUDES "${spdlog_SOURCE_DIR}/include"
        DEPS spdlog::spdlog star_math
)

star_add_module(star_platform
        SOURCES ${STAR_PLATFORM_SRC}
        INCLUDES
                "${SDL3_INCLUDE_DIR}"
                "${imgui_SOURCE_DIR}"
                "${imgui_SOURCE_DIR}/misc/freetype"
                "${freetype_SOURCE_DIR}/include"
        DEPS star_core SDL3::SDL3 imgui freetype
)

star_add_module(star_graphics
        SOURCES ${STAR_GRAPHICS_SRC}
        INCLUDES
                "${BGFX_INCLUDE_DIR}"
                "${BIMG_INCLUDE_DIR}"
                "${BX_INCLUDE_DIR}"
        DEPS star_core bgfx bimg bx shaders
)

star_add_module(star_ecs
        SOURCES ${STAR_ECS_SRC}
        INCLUDES "${flecs_SOURCE_DIR}/include"
        DEPS star_core flecs::flecs_static nlohmann_json::nlohmann_json
)

star_add_module(star_resources
        SOURCES ${STAR_RESOURCES_SRC}
        DEPS star_core star_graphics nlohmann_json::nlohmann_json stb cgltf
)

star_add_module(star_rendering
        SOURCES ${STAR_RENDERING_SRC}
        DEPS star_core star_graphics star_resources star_ecs
)

star_add_module(star_imgui
        SOURCES ${STAR_IMGUI_SRC}
        INCLUDES
                "${imgui_SOURCE_DIR}"
                "${imgui_SOURCE_DIR}/misc/freetype"
                "${freetype_SOURCE_DIR}/include"
                "${BGFX_INCLUDE_DIR}"
                "${BX_INCLUDE_DIR}"
                "${SDL3_INCLUDE_DIR}"
        DEPS star_core star_platform star_graphics star_rendering star_resources imgui freetype bgfx bx SDL3::SDL3
)

star_add_module(star_scene
        SOURCES ${STAR_SCENE_SRC}
        DEPS star_core star_ecs star_rendering
)

star_add_module(star_application
        SOURCES ${STAR_APPLICATION_SRC}
        DEPS star_core star_platform star_graphics star_rendering star_resources star_ecs star_scene star_imgui
)

star_add_module(star_physics
        SOURCES ${STAR_PHYSICS_SRC}
        DEPS star_core star_ecs star_math Jolt
)

star_add_module(star_project
        SOURCES ${STAR_PROJECT_SRC}
        DEPS star_core nlohmann_json::nlohmann_json
)

add_library(${PROJECT_NAME} STATIC
        $<TARGET_OBJECTS:star_core>
        $<TARGET_OBJECTS:star_platform>
        $<TARGET_OBJECTS:star_graphics>
        $<TARGET_OBJECTS:star_ecs>
        $<TARGET_OBJECTS:star_resources>
        $<TARGET_OBJECTS:star_rendering>
        $<TARGET_OBJECTS:star_imgui>
        $<TARGET_OBJECTS:star_scene>
        $<TARGET_OBJECTS:star_application>
        $<TARGET_OBJECTS:star_physics>
        $<TARGET_OBJECTS:star_project>
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
target_link_libraries(${PROJECT_NAME} PUBLIC
        star_core
        star_math
        star_platform
        star_graphics
        star_ecs
        star_resources
        star_rendering
        star_imgui
        star_scene
        star_application
        star_physics
        star_project
        ${CMAKE_DL_LIBS}
)

target_include_directories(${PROJECT_NAME} PUBLIC
        "${SRC_DIR}"
        "${INCLUDE_DIR}"
        "${CMAKE_BINARY_DIR}/include/generated/shaders"
)

target_precompile_headers(star_core PRIVATE "${INCLUDE_DIR}/star/core/common.hpp")
foreach (mod IN ITEMS star_platform star_graphics star_ecs star_resources
                       star_rendering star_imgui star_scene star_application star_project)
    target_precompile_headers(${mod} REUSE_FROM star_core)
endforeach ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(_STAR_ALL_MODULE_FILES
        ${STAR_CORE_SRC} ${STAR_UTILS_SRC} ${STAR_PLATFORM_SRC} ${STAR_GRAPHICS_SRC}
        ${STAR_RESOURCES_SRC} ${STAR_ECS_SRC} ${STAR_RENDERING_SRC}
        ${STAR_IMGUI_SRC}
        ${STAR_SCENE_SRC} ${STAR_APPLICATION_SRC} ${STAR_PHYSICS_SRC}
        ${STAR_PROJECT_SRC}
        ${STAR_MATH_HEADERS}
)
set(_STAR_SRC_TREE_FILES "")
set(_STAR_INCLUDE_TREE_FILES "")
foreach (_f IN LISTS _STAR_ALL_MODULE_FILES)
    if (_f MATCHES "^${SRC_DIR}/")
        list(APPEND _STAR_SRC_TREE_FILES "${_f}")
    elseif (_f MATCHES "^${INCLUDE_DIR}/")
        list(APPEND _STAR_INCLUDE_TREE_FILES "${_f}")
    endif ()
endforeach ()
source_group(TREE "${SRC_DIR}" PREFIX "src" FILES ${_STAR_SRC_TREE_FILES})
source_group(TREE "${INCLUDE_DIR}" PREFIX "include" FILES ${_STAR_INCLUDE_TREE_FILES})

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