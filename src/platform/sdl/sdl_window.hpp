#pragma once
#include "SDL3/SDL.h"
#include "star/platform/window.hpp"

namespace star::platform::sdl {
    class SDLWindow final : public Window {
        using Super = Window;

      public:
        SDLWindow();
        ~SDLWindow() override;

        bool create(const VideoMode& video_mode) override;
        void destroy() override;

        void pool_events() override;
        void* handle() const override;
        void* display_handle() const override;

        PlatformData platform_data() override;

        Vector2 size() const override;
        void set_title(const std::string& title) const override;
        void set_size(int width, int height) override;

      private:
        SDL_Window* m_window = nullptr;
    };
} // namespace star::platform::sdl
