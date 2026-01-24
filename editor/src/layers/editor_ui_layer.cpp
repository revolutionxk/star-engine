#include "editor_ui_layer.hpp"

#include <imgui.h>

#include "star/application/application.hpp"
#include "star/scene/scene_manager.hpp"

namespace star::editor {
    EditorUILayer::EditorUILayer(EditorWindow* editor_window)
        : Layer("EditorUILayer"), m_editor_window(editor_window) {}

    bool EditorUILayer::initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor UI layer");
        return true;
    }

    void EditorUILayer::shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Shutting down editor UI layer");
    }

    void EditorUILayer::on_imgui_render() {
        setup_dockspace();

        render_main_menu_bar();
        render_hierarchy_panel();
        render_inspector_panel();
        render_metrics_panel();
        render_scene_viewport_panel();
        render_console_panel();
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

    void EditorUILayer::render_main_menu_bar() {
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
                ImGui::MenuItem("Hierarchy", nullptr, nullptr);
                ImGui::MenuItem("Inspector", nullptr, nullptr);
                ImGui::MenuItem("Scene", nullptr, nullptr);
                ImGui::MenuItem("Console", nullptr, nullptr);
                ImGui::MenuItem("Metrics", nullptr, nullptr);
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

    void EditorUILayer::render_hierarchy_panel() {
        ImGui::Begin("Hierarchy");

        ImGui::Text("Scene Entities");
        ImGui::Separator();

        ImGui::TextDisabled("(No camera connected to scene)");

        if (ImGui::BeginPopupContextWindow()) {
            if (ImGui::MenuItem("Create Empty Entity")) {
                STAR_LOG_INFO(LogCategory::Editor, "Create entity requested");
            }
            ImGui::EndPopup();
        }

        ImGui::End();
    }

    void EditorUILayer::render_inspector_panel() const {
        ImGui::Begin("Inspector");

        if (m_selected_entity && m_selected_entity.is_alive()) {
            ImGui::Text("Entity: %s", m_selected_entity.name().c_str());
            ImGui::Separator();
            ImGui::Text("Component inspector coming soon");
        } else {
            ImGui::TextDisabled("No entity selected");
        }

        ImGui::End();
    }

    void EditorUILayer::render_metrics_panel() {
        ImGui::Begin("Metrics");

        ImGui::Text("Performance");
        ImGui::Separator();

        const ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("FPS: %.1f", io.Framerate);
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);

        ImGui::Spacing();
        ImGui::Text("Window");
        ImGui::Separator();

        ImGui::End();
    }

    void EditorUILayer::render_scene_viewport_panel() {
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Scene");

        m_viewport_focused = ImGui::IsWindowFocused();
        m_viewport_hovered = ImGui::IsWindowHovered();

        ImVec2 viewport_panel_size = ImGui::GetContentRegionAvail();
        if (viewport_panel_size.x > 0 && viewport_panel_size.y > 0) {
            const u32 new_width = static_cast<u32>(viewport_panel_size.x);
            const u32 new_height = static_cast<u32>(viewport_panel_size.y);

            if (new_width != m_viewport_width || new_height != m_viewport_height) {
                m_viewport_width = new_width;
                m_viewport_height = new_height;
                STAR_LOG_INFO(LogCategory::Editor, "Viewport resized: {}x{}", m_viewport_width, m_viewport_height);
            }
        }
        ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "Scene Viewport (%ux%u)", m_viewport_width,
                           m_viewport_height);

        ImGui::End();
        ImGui::PopStyleVar();
    }

    void EditorUILayer::render_console_panel() {
        ImGui::Begin("Console");

        ImGui::Text("Console Output");
        ImGui::Separator();

        ImGui::TextWrapped("Editor console - logs will appear here");

        ImGui::End();
    }
} // namespace star::editor
