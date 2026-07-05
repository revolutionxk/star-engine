#include "editor_ui_layer.hpp"

#include <ImGuizmo.h>
#include <imgui.h>

#include "../panels/console_panel.hpp"
#include "../panels/hierarchy_panel.hpp"
#include "../panels/inspector_panel.hpp"
#include "../panels/metrics_panel.hpp"
#include "../panels/scene_panel.hpp"
#include "../theme/editor_theme.hpp"
#include "star/application/application.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/imgui/imgui_font_config.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/rendering/passes/debug_render_pass.hpp"
#include "star/rendering/passes/scene_render_pass.hpp"
#include "star/rendering/passes/sky_render_pass.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorUILayer::EditorUILayer(EditorWindow* editor_window)
        : Layer("EditorUILayer"), m_editor_window(editor_window) {}

    bool EditorUILayer::initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor UI layer");

        EditorTheme::apply_theme();

        auto& device = m_editor_window->device();
        m_viewport = std::make_unique<rendering::Viewport>(device);
        m_viewport->set_framebuffer_enabled(true);
        m_viewport->resize(1280, 720);

        auto& render_system = m_editor_window->renderer();
        render_system.set_active_viewport(m_viewport.get());
        render_system.debug_renderer()->set_enabled(true);

        EditorEventBus::instance().subscribe(EditorEventType::EntitySelected, [this](const EditorEvent& event) {
            if (const auto* data = event.get_data<NodeSelectedEvent>())
                m_gizmo_system.set_entity(data->node);
        });
        EditorEventBus::instance().subscribe(EditorEventType::EntityDeselected,
                                             [this](const EditorEvent&) { m_gizmo_system.set_entity(std::nullopt); });

        initialize_panels();

        m_component_inspector.set_resource_manager(&m_editor_window->resources());

        m_input_manager.initialize(application::Application::instance().input_manager());

        auto* active_scene = m_editor_window->scene_manager().get_active_scene();
        if (active_scene) {
            const auto camera_entity = active_scene->find_entity("EditorCamera");
            if (camera_entity.is_valid()) {
                if (const auto* transform = camera_entity.try_get<components::Transform>()) {
                    m_camera_system.initialize(*transform);
                }
            }
        }

        m_camera_system.attach(m_input_manager, &m_editor_window->window());

        return true;
    }

    void EditorUILayer::initialize_panels() {
        auto* hierarchy = m_panel_manager.register_panel<HierarchyPanel>(m_editor_window);
        auto* inspector = m_panel_manager.register_panel<InspectorPanel>(&m_component_inspector);
        auto* scene = m_panel_manager.register_panel<ScenePanel>();
        auto* console = m_panel_manager.register_panel<ConsolePanel>();
        m_panel_manager.register_panel<MetricsPanel>();

        scene->set_viewport(m_viewport.get());
        scene->set_input_manager(&m_input_manager);
        scene->set_gizmo_system(&m_gizmo_system);
    }

    void EditorUILayer::shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Shutting down editor UI layer");
        m_camera_system.detach();
        m_panel_manager.shutdown();
        m_input_manager.shutdown();
        m_viewport.reset();
    }

    void EditorUILayer::update(const f32 dt) {
        m_panel_manager.update_all(dt);

        auto* active_scene = m_editor_window->scene_manager().get_active_scene();
        m_camera_system.update(dt, active_scene->world().native());
    }

    void EditorUILayer::pre_render(const f32 /*dt*/) {
        auto& dr = *m_editor_window->renderer().debug_renderer();

        constexpr Color4 grid_color{0.5f, 0.5f, 0.5f, 0.25f};
        dr.draw_grid({0.0f, 0.0f, 0.0f}, 100, 1.0f, grid_color);
    }

    void EditorUILayer::render() {
        Layer::render();
    }

    void EditorUILayer::on_imgui_render() {
        ImGuizmo::BeginFrame();
        setup_dockspace();
        render_main_menu_bar();
        m_panel_manager.render_all();
    }

    void EditorUILayer::setup_dockspace() {
        static bool dockspace_open = true;
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("DockSpace", &dockspace_open, window_flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleVar(2);

        if (const auto& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        ImGui::End();
    }

    void EditorUILayer::render_main_menu_bar() const {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Scene")) {
                    STAR_LOG_INFO(LogCategory::Editor, "New Scene requested");
                }
                if (ImGui::MenuItem("Open Scene")) {
                    STAR_LOG_INFO(LogCategory::Editor, "Open Scene requested");
                }
                if (ImGui::MenuItem("Save Scene")) {
                    STAR_LOG_INFO(LogCategory::Editor, "Save Scene requested");
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    application::Application::instance().shutdown();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {
                }
                if (ImGui::MenuItem("Redo", "CTRL+Y")) {
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                for (auto* panel : m_panel_manager.get_all_panels()) {
                    bool is_open = panel->is_open();
                    if (ImGui::MenuItem(panel->name().c_str(), nullptr, &is_open)) {
                        panel->set_open(is_open);
                    }
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout")) {
                    STAR_LOG_INFO(LogCategory::Editor, "Reset layout requested");
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About")) {
                    STAR_LOG_INFO(LogCategory::Editor, "About dialog requested");
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }
    }

    void EditorUILayer::on_imgui_init() {
        const std::filesystem::path font_path = "assets/fonts/Roboto-Medium.ttf";

        if (!std::filesystem::exists(font_path)) {
            STAR_LOG_WARN(LogCategory::Editor, "Roboto font not found, using default font");
            platform::ImGuiFontManager::load_default_font(16.0f);
            return;
        }

        platform::FontConfiguration font_config(font_path, 16.0f);
        font_config.freetype_enabled = true;
        font_config.pixel_snap = true;
        font_config.oversample_h = 3;
        font_config.oversample_v = 1;

        platform::ImGuiFontManager::load_font(font_config);
    }
} // namespace star::editor
