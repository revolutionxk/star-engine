#pragma once
#include "star/platform/window.hpp"

namespace star::platform::sdl {
    class SDLWindow : public Window {
      public:
        bool create(const WindowConfig& config) override;
        void destroy() override;
        void pool_events() override;
        Vector2 size() const override;
    };
} // namespace star::platform::sdl
