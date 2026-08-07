#include "editor_ui_layer.hpp"

#include <cstdio>
#include <filesystem>

#include <ImGuizmo.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "../core/icon_registry.hpp"
#include "../panels/console_panel.hpp"
#include "../panels/content_browser_panel.hpp"
#include "../panels/game_panel.hpp"
#include "../panels/hierarchy_panel.hpp"
#include "../panels/inspector_panel.hpp"
#include "../panels/metrics_panel.hpp"
#include "../panels/render_settings_panel.hpp"
#include "../panels/scene_panel.hpp"
#include "../theme/editor_theme.hpp"
#include "star/application/application.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/imgui/imgui_font_config.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/debug_renderer.hpp"
#include "star/rendering/passes/debug_render_pass.hpp"
#include "star/rendering/passes/picking_pass.hpp"
#include "star/rendering/passes/scene_render_pass.hpp"
#include "star/rendering/passes/sky_render_pass.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"
#include "star/scene/scene_serializer.hpp"

namespace star::editor {
    EditorUILayer::EditorUILayer(EditorWindow* editor_window)
        : Layer("EditorUILayer"), m_editor_window(editor_window) {}

    bool EditorUILayer::initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor UI layer");

        EditorTheme::apply_theme();

        auto& device = m_editor_window->device();

        m_scene_viewport = std::make_unique<rendering::Viewport>(device);
        m_scene_viewport->set_framebuffer_enabled(true);
        m_scene_viewport->resize(1280, 720);

        m_game_viewport = std::make_unique<rendering::Viewport>(device);
        m_game_viewport->set_framebuffer_enabled(true);
        m_game_viewport->resize(1280, 720);

        auto& renderer = m_editor_window->renderer();
        renderer.set_active_viewport(m_scene_viewport.get());

        m_scene_view = renderer.add_view({m_scene_viewport.get(), &m_editor_cam, &m_editor_cam_xf, true, true});
        m_game_view = renderer.add_view({m_game_viewport.get(), nullptr, nullptr, false, true});
        renderer.debug_renderer()->set_enabled(true);
        m_picking_pass = renderer.get_render_pass<rendering::PickingPass>();

        EditorEventBus::instance().subscribe(EditorEventType::EntitySelected, [this](const EditorEvent& event) {
            if (const auto* data = event.get_data<NodeSelectedEvent>())
                m_gizmo_system.set_entity(data->node);
        });
        EditorEventBus::instance().subscribe(EditorEventType::EntityDeselected,
                                             [this](const EditorEvent&) { m_gizmo_system.set_entity(std::nullopt); });

        initialize_panels();

        m_component_inspector.set_resource_manager(&m_editor_window->resources());
        m_component_inspector.set_icons(&m_editor_window->icons());

        m_input_manager.initialize(application::Application::instance().input_manager());

        m_editor_cam_xf.position = Vector3{6.0f, 4.0f, 9.0f};
        m_camera_system.initialize(m_editor_cam_xf);
        m_camera_system.attach(m_input_manager, &m_editor_window->window());

        return true;
    }

    void EditorUILayer::initialize_panels() {
        auto* hierarchy = m_panel_manager.register_panel<HierarchyPanel>(m_editor_window);
        auto* inspector = m_panel_manager.register_panel<InspectorPanel>(&m_component_inspector);
        auto* scene = m_panel_manager.register_panel<ScenePanel>();
        auto* game = m_panel_manager.register_panel<GamePanel>();
        auto* console = m_panel_manager.register_panel<ConsolePanel>();
        auto* metrics = m_panel_manager.register_panel<MetricsPanel>();
        m_panel_manager.register_panel<ContentBrowserPanel>(m_editor_window);
        m_panel_manager.register_panel<RenderSettingsPanel>(m_editor_window);

        scene->set_viewport(m_scene_viewport.get());
        scene->set_input_manager(&m_input_manager);
        scene->set_gizmo_system(&m_gizmo_system);
        scene->set_editor_window(m_editor_window);
        EditorEventBus::instance().subscribe(EditorEventType::SceneLoaded,
                                             [this](const EditorEvent&) { m_command_stack.clear(); });
        scene->set_picking_pass(m_picking_pass);
        game->set_viewport(m_game_viewport.get());
        metrics->set_device(&m_editor_window->device());
    }

    void EditorUILayer::shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Shutting down editor UI layer");
        m_camera_system.detach();

        if (m_editor_window) {
            auto& renderer = m_editor_window->renderer();
            renderer.clear_views();
            renderer.set_active_viewport(nullptr);
        }

        m_panel_manager.shutdown();
        m_input_manager.shutdown();
        m_scene_viewport.reset();
        m_game_viewport.reset();
    }

    void EditorUILayer::update(const f32 dt) {
        m_panel_manager.update_all(dt);
        m_camera_system.update(dt, m_editor_cam_xf);
        resolve_pending_pick();
    }

    void EditorUILayer::resolve_pending_pick() const {
        if (!m_picking_pass)
            return;

        const auto hit = m_picking_pass->poll();
        if (!hit.has_value())
            return;

        if (*hit == 0) {
            EditorEventBus::instance().publish({EditorEventType::EntityDeselected, nullptr});
            return;
        }

        auto* scene = m_editor_window->scene_manager().get_active_scene();
        if (!scene)
            return;

        flecs::entity entity = scene->world().native().entity(*hit);
        if (!entity.is_valid())
            return;

        auto selected = NodeSelectedEvent{entity};
        EditorEventBus::instance().publish({EditorEventType::EntitySelected, &selected});
    }

    void EditorUILayer::pre_render(const f32 /*dt*/) {
        auto& dr = *m_editor_window->renderer().debug_renderer();

        constexpr Color4 grid_color{0.5f, 0.5f, 0.5f, 0.25f};
        dr.draw_grid({0.0f, 0.0f, 0.0f}, 100, 1.0f, grid_color);

        if (auto* scene = m_editor_window->scene_manager().get_active_scene())
            m_entity_gizmos.draw(*scene, dr, m_gizmo_system.entity());
    }

    void EditorUILayer::render() {
        Layer::render();
    }

    void EditorUILayer::on_imgui_render() {
        ImGuizmo::BeginFrame();
        render_main_menu_bar();
        render_toolbar();
        setup_dockspace();
        m_panel_manager.render_all();
        render_import_progress();
        render_project_browser();
    }

    void EditorUILayer::render_toolbar() const {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        const f32 height = ImGui::GetFrameHeight() + ImGui::GetStyle().FramePadding.y * 2.0f + 6.0f;

        if (ImGui::BeginViewportSideBar("##EditorToolbar", viewport, ImGuiDir_Up, height,
                                        ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) {
            const PlayState state = m_editor_window->play_state();
            IconRegistry& icons = m_editor_window->icons();

            const f32 btn = ImGui::GetFrameHeight() - ImGui::GetStyle().FramePadding.y * 2.0f;
            const ImVec2 size{btn, btn};
            const f32 spacing = ImGui::GetStyle().ItemSpacing.x;
            const f32 total = btn * 2.0f + spacing;
            ImGui::SetCursorPosX((ImGui::GetContentRegionAvail().x - total) * 0.5f);

            constexpr ImVec4 green{0.42f, 0.82f, 0.5f, 1.0f};
            constexpr ImVec4 amber{0.95f, 0.75f, 0.35f, 1.0f};
            constexpr ImVec4 red{0.9f, 0.45f, 0.42f, 1.0f};
            constexpr ImVec4 transparent{0, 0, 0, 0};

            if (state == PlayState::Playing) {
                if (ImGui::ImageButton("##pause", icons.icon("pause"), size, {0, 0}, {1, 1}, transparent, amber))
                    m_editor_window->pause();
                ImGui::SetItemTooltip("Pause");
            } else {
                if (ImGui::ImageButton("##play", icons.icon("play"), size, {0, 0}, {1, 1}, transparent, green))
                    m_editor_window->play();
                ImGui::SetItemTooltip(state == PlayState::Paused ? "Resume" : "Play");
            }

            ImGui::SameLine();

            ImGui::BeginDisabled(state == PlayState::Editing);
            const ImVec4 stop_tint = state == PlayState::Editing ? ImVec4{0.5f, 0.5f, 0.5f, 1.0f} : red;
            if (ImGui::ImageButton("##stop", icons.icon("square"), size, {0, 0}, {1, 1}, transparent, stop_tint)) {
                m_editor_window->stop();
                EditorEventBus::instance().publish({EditorEventType::EntityDeselected, nullptr});
            }
            ImGui::SetItemTooltip("Stop");
            ImGui::EndDisabled();
        }
        ImGui::End();
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
            const ImGuiID dockspace_id = ImGui::GetID("EditorDockSpace");

            if (ImGui::DockBuilderGetNode(dockspace_id) == nullptr || m_rebuild_layout) {
                m_rebuild_layout = false;
                build_default_layout(dockspace_id);
            }

            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
        }

        if (!GizmoSystem::is_using() && !ImGui::GetIO().WantTextInput) {
            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
                m_command_stack.undo();
            if (ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, ImGuiInputFlags_RouteGlobal) ||
                ImGui::Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, ImGuiInputFlags_RouteGlobal))
                m_command_stack.redo();
        }

        ImGui::End();
    }

    void EditorUILayer::build_default_layout(const unsigned dockspace_id) {
        ImGui::DockBuilderRemoveNode(dockspace_id);
        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

        ImGuiID center = dockspace_id;
        const ImGuiID left = ImGui::DockBuilderSplitNode(center, ImGuiDir_Left, 0.19f, nullptr, &center);
        const ImGuiID right = ImGui::DockBuilderSplitNode(center, ImGuiDir_Right, 0.24f, nullptr, &center);
        const ImGuiID bottom = ImGui::DockBuilderSplitNode(center, ImGuiDir_Down, 0.28f, nullptr, &center);

        ImGui::DockBuilderDockWindow("Hierarchy", left);
        ImGui::DockBuilderDockWindow("Inspector", right);
        ImGui::DockBuilderDockWindow("Metrics", right);
        ImGui::DockBuilderDockWindow("Scene", center);
        ImGui::DockBuilderDockWindow("Game", center);
        ImGui::DockBuilderDockWindow("Console", bottom);
        ImGui::DockBuilderDockWindow("Content Browser", bottom);
        ImGui::DockBuilderFinish(dockspace_id);
    }

    void EditorUILayer::render_main_menu_bar() {
        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project...")) {
                    m_open_browser_requested = true;
                }
                if (ImGui::MenuItem("Open Project...")) {
                    m_open_browser_requested = true;
                }
                if (ImGui::BeginMenu("Recent Projects", !m_editor_window->projects().recent().empty())) {
                    for (const auto& root : m_editor_window->projects().recent()) {
                        if (ImGui::MenuItem(root.filename().string().c_str())) {
                            m_editor_window->open_project(root);
                        }
                    }
                    ImGui::EndMenu();
                }
                ImGui::Separator();

                const bool has_project = m_editor_window->has_project();
                if (ImGui::MenuItem("New Scene", "Ctrl+N", false, has_project)) {
                    m_editor_window->new_scene();
                }
                if (ImGui::MenuItem("Save Scene", "Ctrl+S", false, has_project)) {
                    m_editor_window->save_active_scene();
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Exit")) {
                    application::Application::instance().shutdown();
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                const std::string undo_text =
                    m_command_stack.can_undo() ? "Undo " + std::string{m_command_stack.undo_label()} : "Undo";
                const std::string redo_text =
                    m_command_stack.can_redo() ? "Redo " + std::string{m_command_stack.redo_label()} : "Redo";

                if (ImGui::MenuItem(undo_text.c_str(), "CTRL+Z", false, m_command_stack.can_undo()))
                    m_command_stack.undo();
                if (ImGui::MenuItem(redo_text.c_str(), "CTRL+Y", false, m_command_stack.can_redo()))
                    m_command_stack.redo();
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
                    m_rebuild_layout = true;
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

    void EditorUILayer::render_import_progress() const {
        const auto names = m_editor_window->imports_in_flight_names();
        if (names.empty())
            return;

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        const ImVec2 pos{viewport->WorkPos.x + viewport->WorkSize.x - 16.0f,
                         viewport->WorkPos.y + viewport->WorkSize.y - 16.0f};
        ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(1.0f, 1.0f));
        ImGui::SetNextWindowBgAlpha(0.85f);

        constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
                                           ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                                           ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoDocking;

        if (ImGui::Begin("##ImportProgress", nullptr, flags)) {
            const f32 spin = static_cast<f32>(ImGui::GetTime()) * 2.0f;
            constexpr const char* frames = "|/-\\";
            ImGui::Text("%c  Importing %zu model%s", frames[static_cast<int>(spin) % 4], names.size(),
                        names.size() == 1 ? "" : "s");
            ImGui::Separator();
            for (const auto& name : names) {
                ImGui::BulletText("%s", name.c_str());
            }
        }
        ImGui::End();
    }

    void EditorUILayer::render_project_browser() {
        const bool no_project = !m_editor_window->has_project();

        if ((no_project || m_open_browser_requested) && !ImGui::IsPopupOpen("Project Browser")) {
            ImGui::OpenPopup("Project Browser");
        }
        m_open_browser_requested = false;

        const ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(680.0f, 440.0f), ImGuiCond_Appearing);

        bool keep_open = true;
        if (!ImGui::BeginPopupModal("Project Browser", no_project ? nullptr : &keep_open,
                                    ImGuiWindowFlags_NoCollapse)) {
            return;
        }

        if (m_new_project_location[0] == '\0') {
            const std::string def = ProjectManager::default_projects_dir().string();
            std::snprintf(m_new_project_location, sizeof(m_new_project_location), "%s", def.c_str());
        }

        if (ImGui::BeginTabBar("##ProjectTabs")) {
            if (ImGui::BeginTabItem("New Project")) {
                ImGui::Spacing();
                ImGui::InputText("Name", m_new_project_name, sizeof(m_new_project_name));
                ImGui::InputText("Location", m_new_project_location, sizeof(m_new_project_location));
                const std::string preview =
                    (std::filesystem::path(m_new_project_location) / m_new_project_name).string();
                ImGui::TextDisabled("Creates: %s", preview.c_str());
                ImGui::Spacing();

                const bool can_create = m_new_project_name[0] != '\0' && m_new_project_location[0] != '\0';
                ImGui::BeginDisabled(!can_create);
                if (ImGui::Button("Create", ImVec2(120.0f, 0.0f))) {
                    if (m_editor_window->create_project(m_new_project_location, m_new_project_name)) {
                        m_new_project_name[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndDisabled();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Open Project")) {
                ImGui::Spacing();
                ImGui::TextUnformatted("Recent");
                ImGui::Separator();

                const auto& recent = m_editor_window->projects().recent();
                if (recent.empty()) {
                    ImGui::TextDisabled("No recent projects.");
                }
                for (const auto& root : recent) {
                    const std::string label = root.filename().string() + "##" + root.string();
                    if (ImGui::Selectable(label.c_str())) {
                        if (m_editor_window->open_project(root))
                            ImGui::CloseCurrentPopup();
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("%s", root.string().c_str());
                }

                ImGui::Spacing();
                ImGui::TextUnformatted("Open by path");
                ImGui::InputText("##OpenPath", m_open_project_path, sizeof(m_open_project_path));
                ImGui::SameLine();
                if (ImGui::Button("Open")) {
                    if (m_open_project_path[0] != '\0' && m_editor_window->open_project(m_open_project_path)) {
                        m_open_project_path[0] = '\0';
                        ImGui::CloseCurrentPopup();
                    }
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::EndPopup();
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
