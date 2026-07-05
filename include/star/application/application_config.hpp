#pragma once
#include "command_line_args.hpp"
#include "star/graphics/device.hpp"
#include "star/platform/window.hpp"

namespace star::application {
    struct STAR_EXPORT ApplicationConfig {
        graphics::GraphicsAPI graphics_api = graphics::GraphicsAPI::Auto;

        CommandLineArgs command_line_args;

        std::string log_file = "logs/star_engine.log";

        f32 fixed_timestep = 1.0f / 60.0f;
        u32 max_fps = 0;
        bool allow_multiple_windows = true;
    };
} // namespace star::application
