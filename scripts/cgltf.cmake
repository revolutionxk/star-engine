set(_cgltf_bundled "${bgfx_SOURCE_DIR}/bgfx/3rdparty/cgltf")

add_library(cgltf INTERFACE)
if (EXISTS "${_cgltf_bundled}/cgltf.h")
    target_include_directories(cgltf INTERFACE "${_cgltf_bundled}")
    message(STATUS "cgltf: using bgfx-bundled copy at ${_cgltf_bundled}")
else ()
    include(FetchContent)
    FetchContent_Declare(
            cgltf
            GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
            GIT_TAG master
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(cgltf)
    target_include_directories(cgltf INTERFACE "${cgltf_SOURCE_DIR}")
    message(STATUS "cgltf: fetched from source")
endif ()
