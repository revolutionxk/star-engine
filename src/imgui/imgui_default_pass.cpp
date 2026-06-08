#include "star/imgui/imgui_default_pass.hpp"

#include "imgui_bgfx_renderer.hpp"
#include "imgui_sdl3_backend.hpp"

namespace star::imgui {

    std::unique_ptr<rendering::ImGuiRenderPass> create_default_imgui_pass() {
        return std::make_unique<rendering::ImGuiRenderPass>(
            std::make_unique<platform::sdl::ImGuiSDL3Backend>(),
            std::make_unique<graphics::ImGuiBGFXRenderer>());
    }

} // namespace star::imgui
