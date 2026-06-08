#pragma once

#include "star/imgui/imgui_platform_backend.hpp"

struct SDL_Window;

namespace star::platform::sdl {
    class ImGuiSDL3Backend final : public IImGuiPlatformBackend {
      public:
        ImGuiSDL3Backend() = default;
        ~ImGuiSDL3Backend() override;

        ImGuiSDL3Backend(const ImGuiSDL3Backend&) = delete;
        ImGuiSDL3Backend& operator=(const ImGuiSDL3Backend&) = delete;

        bool initialize(void* window_handle) override;
        void shutdown() override;
        void new_frame() override;

        [[nodiscard]] bool is_initialized() const override {
            return m_initialized;
        }

      private:
        bool m_initialized = false;
    };

} // namespace star::platform::sdl
