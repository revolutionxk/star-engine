#define IMGUI_DEFINE_MATH_OPERATORS
#include "scene_panel.hpp"

#include <imgui.h>

#include "star/rendering/passes/picking_pass.hpp"
#include "star/rendering/viewport.hpp"

namespace star::editor {
    namespace {
        constexpr float FIRST_USE_WIDTH = 960.0f;
        constexpr float FIRST_USE_HEIGHT = 640.0f;
        constexpr u32 MIN_VIEWPORT_WIDTH = 128;
        constexpr u32 MIN_VIEWPORT_HEIGHT = 96;
    } // namespace

    void ScenePanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(FIRST_USE_WIDTH, FIRST_USE_HEIGHT), ImGuiCond_FirstUseEver);
        ImGui::Begin(m_name.c_str(), &m_is_open);

        handle_viewport_resize();
        render_scene_texture();

        const bool gizmo_used =
            m_gizmo && m_viewport && m_gizmo->draw_and_process(m_image_pos, m_image_size, *m_viewport);
        update_focus_state(gizmo_used);

        handle_click_pick();
        render_gizmo_toolbar();

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void ScenePanel::update_focus_state(const bool gizmo_consuming) {
        m_focused = ImGui::IsWindowFocused() && !gizmo_consuming;
        m_hovered = ImGui::IsWindowHovered() && (!m_gizmo || !m_gizmo->is_over());
        if (m_input_manager) {
            m_input_manager->set_viewport_focused(m_focused);
            m_input_manager->set_viewport_hovered(m_hovered);
        }
    }

    void ScenePanel::handle_viewport_resize() const {
        if (!m_viewport)
            return;
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        if (avail.x <= 0 || avail.y <= 0)
            return;
        const auto nw = static_cast<u32>(avail.x);
        const auto nh = static_cast<u32>(avail.y);
        if (nw < MIN_VIEWPORT_WIDTH || nh < MIN_VIEWPORT_HEIGHT)
            return;
        if (nw != m_viewport->width() || nh != m_viewport->height())
            m_viewport->resize(nw, nh);
    }

    void ScenePanel::render_scene_texture() {
        m_image_pos = {};
        m_image_size = {};

        if (!m_viewport || !m_viewport->is_framebuffer_enabled()) {
            render_placeholder();
            return;
        }

        const auto color_texture = m_viewport->get_color_texture();
        if (!color_texture.is_valid()) {
            render_error_message();
            return;
        }

        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const bool y_flip = m_viewport->needs_uv_y_flip();
        const ImVec2 uv0 = y_flip ? ImVec2(0.0f, 1.0f) : ImVec2(0.0f, 0.0f);
        const ImVec2 uv1 = y_flip ? ImVec2(1.0f, 0.0f) : ImVec2(1.0f, 1.0f);
        ImGui::Image(color_texture.id, avail, uv0, uv1);
        m_image_pos = ImGui::GetItemRectMin();
        m_image_size = ImGui::GetItemRectSize();
    }

    void ScenePanel::handle_click_pick() const {
        if (!m_hovered || !m_viewport || !m_picking_pass)
            return;
        if (m_image_size.x <= 0.0f || m_image_size.y <= 0.0f)
            return;
        if (GizmoSystem::is_over() || GizmoSystem::is_using())
            return;

        if (!ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::GetIO().KeyAlt)
            return;

        const ImVec2 mouse = ImGui::GetMousePos();
        const f32 u = (mouse.x - m_image_pos.x) / m_image_size.x;
        const f32 v = (mouse.y - m_image_pos.y) / m_image_size.y;
        if (u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
            return;
        
        m_picking_pass->request(static_cast<u32>(u * static_cast<f32>(m_viewport->width())),
                                static_cast<u32>(v * static_cast<f32>(m_viewport->height())));
    }

    void ScenePanel::render_gizmo_toolbar() const {
        if (!m_gizmo)
            return;

        constexpr float btn = 30.0f;
        constexpr float gap = 4.0f;

        const ImVec2 pos(ImGui::GetWindowContentRegionMax().x - (btn * 4 + gap * 3 + 10.0f),
                         ImGui::GetWindowContentRegionMin().y + 10.0f);
        ImGui::SetCursorPos(pos);

        auto op_button = [&](const char* label, GizmoSystem::Operation op) {
            const bool active = m_gizmo->operation() == op;
            if (active)
                ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            if (ImGui::Button(label, ImVec2(btn, btn)))
                m_gizmo->set_operation(op);
            if (active)
                ImGui::PopStyleColor();
            ImGui::SameLine(0.0f, gap);
        };

        op_button("T", GizmoSystem::Operation::Translate);
        op_button("R", GizmoSystem::Operation::Rotate);
        op_button("S", GizmoSystem::Operation::Scale);

        const bool world = m_gizmo->space() == GizmoSystem::Space::World;
        if (world)
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        if (ImGui::Button(world ? "W" : "L", ImVec2(btn, btn)))
            m_gizmo->set_space(world ? GizmoSystem::Space::Local : GizmoSystem::Space::World);
        if (world)
            ImGui::PopStyleColor();
    }

    void ScenePanel::render_placeholder() {
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPos({avail.x * 0.5f, avail.y * 0.5f});
        ImGui::TextDisabled("Scene viewport not available");
    }

    void ScenePanel::render_error_message() {
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        ImGui::SetCursorPos({avail.x * 0.5f, avail.y * 0.5f});
        ImGui::TextColored({1.0f, 0.3f, 0.3f, 1.0f}, "Texture not available");
    }
} // namespace star::editor
