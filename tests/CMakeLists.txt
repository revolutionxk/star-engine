cmake_minimum_required(VERSION 3.29)

include("${CMAKE_SOURCE_DIR}/scripts/catch2.cmake")

enable_testing()

set(TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB_RECURSE TEST_SRC_FILES CONFIGURE_DEPENDS
    "${TEST_DIR}/src/**.cpp"
    "${TEST_DIR}/src/**.hpp"
    "${TEST_DIR}/src/**.h"
)

set(TEST_TARGET "star_engine_tests")

add_executable(${TEST_TARGET} ${TEST_SRC_FILES})

set_target_properties(${TEST_TARGET} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(${TEST_TARGET} PRIVATE
    "${SRC_DIR}"
    "${INCLUDE_DIR}"
    "${TEST_DIR}/src"
)

target_precompile_headers(${TEST_TARGET} PRIVATE "${INCLUDE_DIR}/star/core/common.hpp")

target_link_libraries(${TEST_TARGET} PRIVATE
    ${PROJECT_NAME}
    Catch2::Catch2WithMain
)

catch_discover_tests(${TEST_TARGET})

message(STATUS "Configured test: ${TEST_TARGET}")
