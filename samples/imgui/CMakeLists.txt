cmake_minimum_required(VERSION 3.29)

get_filename_component(SAMPLE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(SAMPLE_TARGET "sample_${SAMPLE_NAME}")

file(GLOB_RECURSE SAMPLE_SRC_FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

if (WIN32)
    add_executable(${SAMPLE_TARGET} WIN32 ${SAMPLE_SRC_FILES})
else ()
    add_executable(${SAMPLE_TARGET} ${SAMPLE_SRC_FILES})
endif ()

set_property(TARGET ${SAMPLE_TARGET} PROPERTY CXX_STANDARD 23)

target_include_directories(${SAMPLE_TARGET} PRIVATE
        "${SRC_DIR}"
        "${INCLUDE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_precompile_headers(${SAMPLE_TARGET} PRIVATE "${INCLUDE_DIR}/star/core/common.hpp")

target_link_libraries(${SAMPLE_TARGET} PRIVATE ${PROJECT_NAME})

add_custom_command(TARGET ${SAMPLE_TARGET} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_SOURCE_DIR}/assets" "${CMAKE_CURRENT_BINARY_DIR}/assets"
)

if (WIN32)
    add_custom_command(
            TARGET ${SAMPLE_TARGET} POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:SDL3::SDL3>" "$<TARGET_FILE_DIR:${SAMPLE_TARGET}>"
            VERBATIM
    )
endif ()

message(STATUS "Configured sample: ${SAMPLE_TARGET}")
