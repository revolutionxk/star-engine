#pragma once

namespace star::application {
    struct ApplicationConfig {
        std::string title{"Star Engine"};
        int width = 1024;
        int height = 768;
        bool vsync = true;
        bool resizable = true;
        bool fullscreen = false;
    };
} // namespace star::application
