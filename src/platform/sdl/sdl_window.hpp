#pragma once
#include "SDL3/SDL.h"
#include "star/platform/window.hpp"

namespace star::platform {
    class Input;
} // namespace star::platform

namespace star::platform::sdl {
    class SDLInput;
}

namespace star::platform::sdl {
    class SDLWindow final : public Window {
        using Super = Window;

      public:
        SDLWindow();
        ~SDLWindow() override;

        bool create(const WindowConfiguration& config) override;
        void destroy() override;

        void poll_events() override;
        void set_resize_callback(ResizeCallback callback) override;
        void* handle() const override;
        void* native_handle() const override;
        void* display_handle() const override;
        NativeWindowHandleType native_handle_type() const override;

        PlatformData platform_data() override;

        Vector2 size() const override;
        f32 pixel_density() const override;
        void set_title(const std::string& title) const override;
        void set_size(int width, int height) override;

        void set_input_manager(Input* input_manager) override;
        void set_relative_mouse_mode(bool enabled) override;

      private:
        static int s_ref_count;

        SDL_Window* m_window = nullptr;
        SDLInput* m_input_manager = nullptr;
#ifdef STAR_PLATFORM_MACOS
        SDL_MetalView m_metal_view = nullptr;
#endif
    };
} // namespace star::platform::sdl
