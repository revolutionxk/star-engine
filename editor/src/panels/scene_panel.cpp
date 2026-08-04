#define IMGUI_DEFINE_MATH_OPERATORS
#include "scene_panel.hpp"

#include <cmath>

#include <imgui.h>

#include "../core/editor_events.hpp"
#include "../core/entity_gizmos.hpp"
#include "../core/icon_registry.hpp"
#include "../editor_window.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/passes/picking_pass.hpp"
#include "star/rendering/viewport.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    constexpr float FIRST_USE_WIDTH = 960.0f;
    constexpr float FIRST_USE_HEIGHT = 640.0f;
    constexpr u32 MIN_VIEWPORT_WIDTH = 128;
    constexpr u32 MIN_VIEWPORT_HEIGHT = 96;
    constexpr float ICON_HALF = 13.0f;

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

        render_entity_icons();
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
        const ImVec2 fb_scale = ImGui::GetIO().DisplayFramebufferScale;
        const auto nw = static_cast<u32>(avail.x * fb_scale.x);
        const auto nh = static_cast<u32>(avail.y * fb_scale.y);
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

    void ScenePanel::render_entity_icons() {
        m_icon_click_consumed = false;
        if (!m_viewport || !m_editor_window || m_image_size.x <= 0.0f || m_image_size.y <= 0.0f)
            return;

        auto* scene = m_editor_window->scene_manager().get_active_scene();
        if (!scene)
            return;

        const Matrix4 view_proj = m_viewport->cur_view_proj();
        ImDrawList* dl = ImGui::GetWindowDrawList();
        IconRegistry& icons = m_editor_window->icons();

        const auto& sel_opt = m_gizmo ? m_gizmo->entity() : std::optional<flecs::entity>{};
        const u64 sel = sel_opt && sel_opt->is_valid() ? sel_opt->id() : 0;

        const bool can_click = m_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::GetIO().KeyAlt &&
                               !GizmoSystem::is_over() && !GizmoSystem::is_using();
        const ImVec2 mouse = ImGui::GetMousePos();
        flecs::entity clicked{};
        f32 best_dist = 1e30f;

        const auto draw_icon = [&](const flecs::entity e, const char* icon_name, const ImVec4& tint) {
            const Vector3 pos = EntityGizmos::world_position(e);
            const Vector4 clip = view_proj * Vector4{pos.x, pos.y, pos.z, 1.0f};
            if (clip.w <= 1e-4f)
                return;
            const f32 ndc_x = clip.x / clip.w;
            const f32 ndc_y = clip.y / clip.w;
            if (ndc_x < -1.2f || ndc_x > 1.2f || ndc_y < -1.2f || ndc_y > 1.2f)
                return;

            const f32 sx = m_image_pos.x + (ndc_x * 0.5f + 0.5f) * m_image_size.x;
            const f32 sy = m_image_pos.y + (1.0f - (ndc_y * 0.5f + 0.5f)) * m_image_size.y;

            const bool selected = e.id() == sel;
            const ImVec4 col = selected ? ImVec4{1.0f, 0.85f, 0.25f, 1.0f} : tint;
            dl->AddImage(icons.icon(icon_name), {sx - ICON_HALF, sy - ICON_HALF}, {sx + ICON_HALF, sy + ICON_HALF},
                         {0, 0}, {1, 1}, ImGui::ColorConvertFloat4ToU32(col));

            if (can_click && std::abs(mouse.x - sx) <= ICON_HALF && std::abs(mouse.y - sy) <= ICON_HALF) {
                const f32 d = (mouse.x - sx) * (mouse.x - sx) + (mouse.y - sy) * (mouse.y - sy);
                if (d < best_dist) {
                    best_dist = d;
                    clicked = e;
                }
            }
        };

        flecs::world& world = scene->world().native();
        world.query<const components::Camera, const components::Transform>().each(
            [&](const flecs::entity e, const components::Camera&, const components::Transform&) {
                draw_icon(e, "camera", ImVec4{0.6f, 0.85f, 1.0f, 1.0f});
            });
        world.query<const components::Light, const components::Transform>().each(
            [&](const flecs::entity e, const components::Light& light, const components::Transform&) {
                const char* icon = light.type == components::Light::Type::Directional ? "sun" : "lightbulb";
                draw_icon(e, icon, ImVec4{1.0f, 0.9f, 0.55f, 1.0f});
            });

        if (can_click && clicked.is_valid()) {
            m_icon_click_consumed = true;
            auto selected = NodeSelectedEvent{clicked};
            EditorEventBus::instance().publish({EditorEventType::EntitySelected, &selected});
        }
    }

    void ScenePanel::handle_click_pick() const {
        if (m_icon_click_consumed)
            return;
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
        if (!m_gizmo || !m_editor_window)
            return;

        IconRegistry& icons = m_editor_window->icons();
        const ImGuiStyle& style = ImGui::GetStyle();
        constexpr float img = 18.0f;
        constexpr float gap = 4.0f;
        const float bw = img + style.FramePadding.x * 2.0f;

        const ImVec2 pos(ImGui::GetWindowContentRegionMax().x - (bw * 4 + gap * 3 + 10.0f),
                         ImGui::GetWindowContentRegionMin().y + 10.0f);
        ImGui::SetCursorPos(pos);

        const ImVec4 active_tint{0.45f, 0.72f, 1.0f, 1.0f};
        const ImVec4 idle_tint{0.82f, 0.82f, 0.86f, 1.0f};

        auto op_button = [&](const char* id, const char* icon, GizmoSystem::Operation op, const char* tip) {
            const bool active = m_gizmo->operation() == op;
            if (ImGui::ImageButton(id, icons.icon(icon), ImVec2(img, img), {0, 0}, {1, 1}, {0, 0, 0, 0},
                                   active ? active_tint : idle_tint))
                m_gizmo->set_operation(op);
            ImGui::SetItemTooltip("%s", tip);
            ImGui::SameLine(0.0f, gap);
        };

        op_button("##gizmo_t", "move", GizmoSystem::Operation::Translate, "Translate");
        op_button("##gizmo_r", "rotate-3d", GizmoSystem::Operation::Rotate, "Rotate");
        op_button("##gizmo_s", "scale-3d", GizmoSystem::Operation::Scale, "Scale");

        const bool world = m_gizmo->space() == GizmoSystem::Space::World;
        if (ImGui::ImageButton("##gizmo_space", icons.icon(world ? "globe" : "axis-3d"), ImVec2(img, img), {0, 0},
                               {1, 1}, {0, 0, 0, 0}, idle_tint))
            m_gizmo->set_space(world ? GizmoSystem::Space::Local : GizmoSystem::Space::World);
        ImGui::SetItemTooltip(world ? "World space" : "Local space");
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
