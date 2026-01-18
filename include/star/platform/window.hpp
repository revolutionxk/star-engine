#pragma once

namespace star::platform {
    struct WindowConfig {
        const char* title = "Star Engine";
        int width = 800;
        int height = 600;
        bool resizable = true;
        bool fullscreen = false;
    };

    class Window {
      public:
        virtual ~Window() = default;

        virtual bool create(const WindowConfig& config) = 0;
        virtual void destroy() = 0;
        virtual void pool_events() = 0;

        virtual void* handle() const = 0;
        virtual Vector2 size() const = 0;

        static std::unique_ptr<Window> create_platform_window();
    };
} // namespace star::platform
