cmake_minimum_required(VERSION 3.29)

file(GLOB sample_dirs LIST_DIRECTORIES true CONFIGURE_DEPENDS RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/*")

foreach(sample_dir ${sample_dirs})
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${sample_dir}")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${sample_dir}/CMakeLists.txt")
            add_subdirectory(${sample_dir})
            message(STATUS "Added sample subdirectory: ${sample_dir}")
        endif()
    endif()
endforeach()
