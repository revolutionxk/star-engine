#pragma once

#include <memory>

#include "star/imgui/imgui_render_pass.hpp"

namespace star::platform {
    class Window;
}

namespace star::imgui {
    [[nodiscard]] std::unique_ptr<rendering::ImGuiRenderPass> create_default_imgui_pass();
} // namespace star::imgui
