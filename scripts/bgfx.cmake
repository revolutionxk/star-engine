include(FetchContent)

message("BGFX")

set(BGFX_BUILD_TOOLS ON CACHE BOOL "Build bgfx tools" FORCE)
set(BGFX_BUILD_TOOLS_SHADER ON CACHE BOOL "Build bgfx shader tools" FORCE)
set(BGFX_BUILD_TOOLS_TEXTURE ON CACHE BOOL "Build bgfx texture tools" FORCE)
set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Build bgfx examples" FORCE)

FetchContent_Declare(
        bgfx
        GIT_REPOSITORY https://github.com/bkaradzic/bgfx.cmake
        GIT_TAG v1.135.9062-502
        GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(bgfx)