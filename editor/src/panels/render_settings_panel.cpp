#include "render_settings_panel.hpp"

#include <imgui.h>

#include "../editor_window.hpp"
#include "star/rendering/passes/shadow_pass.hpp"
#include "star/rendering/renderer.hpp"

namespace star::editor {
    void RenderSettingsPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        auto* shadow_pass = m_editor_window->renderer().get_render_pass<rendering::ShadowPass>();
        if (shadow_pass) {
            ImGui::SeparatorText("Directional Shadows");

            auto& s = shadow_pass->settings();
            ImGui::Checkbox("Enabled", &s.enabled);

            ImGui::BeginDisabled(!s.enabled);
            ImGui::DragFloat("Depth Bias", &s.depth_bias, 0.0001f, 0.0f, 0.02f, "%.4f");
            ImGui::SetItemTooltip("Raise to remove shadow acne (stripes); lower if shadows detach (peter-panning).");
            ImGui::DragFloat("Coverage", &s.coverage, 1.0f, 5.0f, 200.0f, "%.0f");
            ImGui::SetItemTooltip("Orthographic half-size around the camera. Larger = more area but softer/blockier.");
            ImGui::EndDisabled();
        } else {
            ImGui::TextDisabled("Shadow pass unavailable");
        }

        ImGui::End();
    }
} // namespace star::editor
