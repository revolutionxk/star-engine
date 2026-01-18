#pragma once
#include "star/platform/window.hpp"

namespace star::platform::sdl {
    class SDLWindow final : public Window {
      public:
        bool create(const WindowConfig& config) override;
        void destroy() override;
        void pool_events() override;
        void* handle() const override;
        Vector2 size() const override;

      private:
        SDL_Window* m_window = nullptr;
    };
} // namespace star::platform::sdl
