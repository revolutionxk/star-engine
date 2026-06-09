include(FetchContent)

FetchContent_Declare(
        imguizmo
        GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo.git
        GIT_TAG master
        GIT_PROGRESS TRUE
)

message("ImGuizmo")
FetchContent_MakeAvailable(imguizmo)

if (NOT TARGET imguizmo)
    add_library(imguizmo STATIC
            "${imguizmo_SOURCE_DIR}/ImGuizmo.cpp"
    )
endif ()

target_include_directories(imguizmo PUBLIC
        "${imguizmo_SOURCE_DIR}"
        "${imgui_SOURCE_DIR}"
)

target_link_libraries(imguizmo PUBLIC imgui)
set_property(TARGET imguizmo PROPERTY CXX_STANDARD 23)