#pragma once

namespace star::platform {
    struct WindowConfig {
        std::string title = "Star Engine";
        int width = 800;
        int height = 600;
        bool resizable = true;
        bool fullscreen = false;
    };

    class Window {
      public:
        virtual ~Window() = default;

        virtual bool create(const WindowConfig& config);
        virtual void destroy() = 0;
        virtual void pool_events() = 0;

        virtual bool is_opened() const {
            return m_opened;
        }

        virtual void* handle() const = 0;
        virtual Vector2 size() const = 0;
        virtual void set_title(const std::string& title) const = 0;
        virtual void set_size(int width, int height) = 0;

        static std::unique_ptr<Window> create_platform_window();

      protected:
        bool m_opened = true;
    };
} // namespace star::platform
