#pragma once
#include <functional>

#include "imgui_platform_backend.hpp"
#include "platform_data.hpp"
#include "star/core/common.hpp"
#include "star/graphics/device_context.hpp"
#include "window_config.hpp"

namespace star::platform {
    class Input;
    class Window {
      public:
        virtual ~Window() = default;

        virtual bool create(const WindowConfiguration& config);
        virtual void destroy() = 0;
        virtual void pool_events() = 0;

        virtual bool is_opened() const {
            return m_opened;
        }

        virtual void* handle() const = 0;
        virtual void* native_handle() const = 0;

        virtual void* display_handle() const {
            return nullptr;
        }

        virtual PlatformData platform_data() = 0;

        virtual Vector2 size() const = 0;
        virtual void set_title(const std::string& title) const = 0;
        virtual void set_size(int width, int height) = 0;

        virtual std::unique_ptr<IImGuiPlatformBackend> create_imgui_backend() const {
            return nullptr;
        }

        virtual void set_input_manager(Input* input_manager) {}

        using ResizeCallback = std::function<void(u32, u32)>;

        virtual void set_resize_callback(ResizeCallback callback) {
            m_resize_callback = std::move(callback);
        }

        static std::unique_ptr<Window> create();

      protected:
        bool m_opened = true;
        WindowConfiguration m_configuration{};
        std::shared_ptr<graphics::DeviceContext> m_device_context{};
        ResizeCallback m_resize_callback;
    };
} // namespace star::platform
