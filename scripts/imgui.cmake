include(FetchContent)
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG docking
        GIT_PROGRESS TRUE
)
message("ImGui")
FetchContent_MakeAvailable(imgui)
file(GLOB SRC_IMGUI
        "${imgui_SOURCE_DIR}/*.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp"
        "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
        "${imgui_SOURCE_DIR}/misc/freetype/imgui_freetype.cpp"
)
add_library(imgui STATIC ${SRC_IMGUI})
source_group(TREE ${imgui_SOURCE_DIR} PREFIX "imgui" FILES ${SRC_IMGUI})
target_include_directories(imgui PRIVATE
        "${imgui_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}/backends"
        "${imgui_SOURCE_DIR}/misc/cpp"
        "${imgui_SOURCE_DIR}/misc/freetype"
        "${freetype_SOURCE_DIR}/include"
)
target_compile_definitions(imgui PRIVATE IMGUI_ENABLE_FREETYPE)
target_link_libraries(imgui PRIVATE SDL3::SDL3 freetype)
set_property(TARGET imgui PROPERTY CXX_STANDARD 23)