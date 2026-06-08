#include "imgui_sdl3_backend.hpp"

#include <SDL3/SDL.h>
#include <backends/imgui_impl_sdl3.h>

#include "star/core/logger.hpp"

namespace star::platform::sdl {

    ImGuiSDL3Backend::~ImGuiSDL3Backend() {
        shutdown();
    }

    bool ImGuiSDL3Backend::initialize(void* window_handle) {
        if (m_initialized) {
            STAR_LOG_WARN(LogCategory::Platform, "ImGuiSDL3Backend already initialized");
            return true;
        }

        if (!window_handle) {
            STAR_LOG_ERROR(LogCategory::Platform, "Invalid window handle for ImGui SDL3 backend");
            return false;
        }

        auto* sdl_window = static_cast<SDL_Window*>(window_handle);

        STAR_LOG_INFO(LogCategory::Platform, "Initializing ImGui SDL3 backend...");

        if (!ImGui_ImplSDL3_InitForOther(sdl_window)) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to initialize ImGui SDL3 backend");
            return false;
        }

        m_initialized = true;
        STAR_LOG_INFO(LogCategory::Platform, "ImGui SDL3 backend initialized successfully");

        return true;
    }

    void ImGuiSDL3Backend::shutdown() {
        if (!m_initialized) {
            return;
        }

        STAR_LOG_INFO(LogCategory::Platform, "Shutting down ImGui SDL3 backend...");
        ImGui_ImplSDL3_Shutdown();
        m_initialized = false;
    }

    void ImGuiSDL3Backend::new_frame() {
        if (!m_initialized) {
            return;
        }

        ImGui_ImplSDL3_NewFrame();
    }

} // namespace star::platform::sdl
