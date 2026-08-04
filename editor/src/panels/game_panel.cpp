#define IMGUI_DEFINE_MATH_OPERATORS
#include "game_panel.hpp"

#include <imgui.h>

#include "star/rendering/viewport.hpp"

namespace star::editor {
    constexpr float FIRST_USE_WIDTH = 640.0f;
    constexpr float FIRST_USE_HEIGHT = 360.0f;
    constexpr u32 MIN_VIEWPORT_WIDTH = 128;
    constexpr u32 MIN_VIEWPORT_HEIGHT = 96;

    void centered_text(const char* text, const ImVec4& color) {
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const float tw = ImGui::CalcTextSize(text).x;
        ImGui::SetCursorPos({(avail.x - tw) * 0.5f, avail.y * 0.5f});
        ImGui::TextColored(color, "%s", text);
    }

    void GamePanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(FIRST_USE_WIDTH, FIRST_USE_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin(m_name.c_str(), &m_is_open);

        handle_viewport_resize();

        if (!m_viewport || !m_viewport->is_framebuffer_enabled()) {
            centered_text("Game viewport not available", {0.6f, 0.6f, 0.6f, 1.0f});
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }

        const auto color_texture = m_viewport->get_color_texture();
        if (!color_texture.is_valid()) {
            centered_text("Texture not available", {1.0f, 0.3f, 0.3f, 1.0f});
            ImGui::End();
            ImGui::PopStyleVar();
            return;
        }

        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const ImVec2 cursor = ImGui::GetCursorPos();

        const bool y_flip = rendering::Viewport::needs_uv_y_flip();
        const ImVec2 uv0 = y_flip ? ImVec2(0.0f, 1.0f) : ImVec2(0.0f, 0.0f);
        const ImVec2 uv1 = y_flip ? ImVec2(1.0f, 0.0f) : ImVec2(1.0f, 1.0f);
        ImGui::Image(color_texture.id, avail, uv0, uv1);
        
        if (!m_viewport->has_camera()) {
            ImGui::SetCursorPos(cursor);
            centered_text("No camera in scene", {1.0f, 0.7f, 0.3f, 1.0f});
        }

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void GamePanel::handle_viewport_resize() const {
        if (!m_viewport)
            return;
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x <= 0 || avail.y <= 0)
            return;
        const ImVec2 fb_scale = ImGui::GetIO().DisplayFramebufferScale;
        const auto nw = static_cast<u32>(avail.x * fb_scale.x);
        const auto nh = static_cast<u32>(avail.y * fb_scale.y);
        if (nw < MIN_VIEWPORT_WIDTH || nh < MIN_VIEWPORT_HEIGHT)
            return;
        if (nw != m_viewport->width() || nh != m_viewport->height())
            m_viewport->resize(nw, nh);
    }
} // namespace star::editor
