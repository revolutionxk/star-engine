cmake_minimum_required(VERSION 3.29)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/editor/src")

file(GLOB_RECURSE EDITOR_SRC_FILES CONFIGURE_DEPENDS
        "${SRC_DIR}/**.cpp"
        "${SRC_DIR}/**.hpp"
        "${SRC_DIR}/**.h"
)

set(EDITOR_NAME "${PROJECT_NAME}_editor")

option(ENABLE_CONSOLE_APPLICATION "Build the editor as a console application" OFF)

if (WIN32 AND NOT ENABLE_CONSOLE_APPLICATION)
    add_executable(${EDITOR_NAME} WIN32 ${EDITOR_SRC_FILES})
else ()
    add_executable(${EDITOR_NAME} ${EDITOR_SRC_FILES})
endif ()

if (ENABLE_CONSOLE_APPLICATION)
    target_compile_definitions(${EDITOR_NAME} PUBLIC STAR_CONSOLE_APPLICATION)
endif ()

set_target_properties(${EDITOR_NAME} PROPERTIES
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "editor" FILES ${EDITOR_SRC_FILES})

target_precompile_headers(${EDITOR_NAME} PRIVATE "${INCLUDE_DIR}/star/core/common.hpp")

target_link_libraries(${EDITOR_NAME} PUBLIC
        ${PROJECT_NAME}
        imguizmo
        lunasvg::lunasvg
        ${CMAKE_DL_LIBS}
)

target_include_directories(${EDITOR_NAME} PRIVATE
        "${SRC_DIR}"
        "${INCLUDE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${BGFX_INCLUDE_DIR}"
        "${BIMG_INCLUDE_DIR}"
        "${BX_INCLUDE_DIR}"
        "${SDL3_INCLUDE_DIR}"
        "${spdlog_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/misc/freetype"
        "${freetype_SOURCE_DIR}/include"
        "${CMAKE_BINARY_DIR}/include/generated/shaders"
)

add_custom_command(TARGET ${EDITOR_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${CMAKE_CURRENT_BINARY_DIR}/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
)

if (MSVC AND WIN32 AND NOT MSVC_VERSION VERSION_LESS 142)
    target_link_options(${EDITOR_NAME} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
    target_compile_options(${EDITOR_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI>)
endif ()

if (WIN32)
    add_custom_command(
            TARGET ${EDITOR_NAME} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:${EDITOR_NAME}>"
            VERBATIM
    )
endif ()

message(STATUS "Configured editor executable: ${EDITOR_NAME}")
