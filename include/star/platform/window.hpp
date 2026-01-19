#pragma once
#include "platform_data.hpp"
#include "star/graphics/device_context.hpp"
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

        virtual void* display_handle() const {
            return nullptr;
        }

        graphics::DeviceContext* device_context() {
            return m_device_context.get();
        }

        const graphics::DeviceContext* device_context() const {
            return m_device_context.get();
        }

        void set_device_context(std::unique_ptr<graphics::DeviceContext> context) {
            m_device_context = std::move(context);
        }

        virtual PlatformData platform_data() = 0;

        virtual Vector2 size() const = 0;
        virtual void set_title(const std::string& title) const = 0;
        virtual void set_size(int width, int height) = 0;

        static std::unique_ptr<Window> create();

      protected:
        bool m_opened = true;
        VideoMode m_video_mode;
        std::unique_ptr<graphics::DeviceContext> m_device_context{};
    };
} // namespace star::platform
