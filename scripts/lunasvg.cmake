include(FetchContent)

set(LUNASVG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
        lunasvg
        GIT_REPOSITORY https://github.com/sammycage/lunasvg.git
        GIT_TAG master
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(lunasvg)
