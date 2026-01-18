#pragma once
#include "star/platform/window.hpp"

namespace star::application {
    struct ApplicationConfig {
        const char* title = "Star Engine";
        i32 width = 1280;
        i32 height = 720;

        platform::WindowConfig window{
            .title = title,
            .width = width,
            .height = height,
            .resizable = true,
            .fullscreen = false,
        };
    };
} // namespace star::application
