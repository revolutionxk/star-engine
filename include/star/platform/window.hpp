#pragma once
#include "window_config.hpp"

namespace star::platform {
    class Window {
      public:
        virtual ~Window() = default;

        virtual bool create(const VideoMode& video_mode);
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
        VideoMode m_video_mode;
    };
} // namespace star::platform
