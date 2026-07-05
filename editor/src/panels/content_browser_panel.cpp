#include "content_browser_panel.hpp"

#include <algorithm>
#include <vector>

#include <imgui.h>

#include "../core/editor_events.hpp"
#include "../editor_window.hpp"
#include "star/project/project.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"
#include "star/scene/scene_serializer.hpp"

namespace star::editor {
    ContentBrowserPanel::ContentBrowserPanel(EditorWindow* editor_window)
        : Panel("Content Browser"), m_editor_window(editor_window), m_root(std::filesystem::current_path()),
          m_current(std::filesystem::current_path()) {}

    void ContentBrowserPanel::on_imgui_render() {
        if (!m_is_open)
            return;
        
        if (const auto* project = m_editor_window->projects().active(); project && m_root != project->assets_dir()) {
            m_root = project->assets_dir();
            m_current = m_root;
        }

        ImGui::Begin(m_name.c_str(), &m_is_open);

        if (!m_editor_window->has_project()) {
            ImGui::TextDisabled("No project open.");
            ImGui::End();
            return;
        }

        ImGui::BeginDisabled(m_current == m_root);
        if (ImGui::Button("<"))
            m_current = m_current.parent_path();
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("%s", m_current.lexically_relative(m_root).string().c_str());
        ImGui::Separator();

        std::error_code ec;
        if (!std::filesystem::is_directory(m_current, ec)) {
            m_current = m_root;
            ImGui::End();
            return;
        }

        std::vector<std::filesystem::directory_entry> directories;
        std::vector<std::filesystem::directory_entry> files;
        for (const auto& entry : std::filesystem::directory_iterator(m_current, ec)) {
            if (entry.is_directory(ec))
                directories.push_back(entry);
            else
                files.push_back(entry);
        }

        const auto by_name = [](const auto& a, const auto& b) { return a.path().filename() < b.path().filename(); };
        std::ranges::sort(directories, by_name);
        std::ranges::sort(files, by_name);

        for (const auto& entry : directories) {
            const auto label = "[dir]  " + entry.path().filename().string();
            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick) &&
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                m_current = entry.path();
            }
        }

        for (const auto& entry : files) {
            const auto name = entry.path().filename().string();
            const bool is_scene = name.ends_with(project::layout::SCENE_EXTENSION);
            const bool is_material = name.ends_with(project::layout::MATERIAL_EXTENSION);

            if (is_scene)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.72f, 1.0f, 1.0f));
            else if (is_material)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.68f, 0.35f, 1.0f));

            const bool clicked = ImGui::Selectable(name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick);

            if (is_scene || is_material)
                ImGui::PopStyleColor();

            if (clicked && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && is_scene) {
                (void)m_editor_window->open_scene_file(entry.path());
            } else if (clicked && is_material) {
                if (const auto* project = m_editor_window->projects().active()) {
                    std::string relative = project->to_relative(entry.path()).generic_string();
                    EditorEventBus::instance().publish({EditorEventType::MaterialAssetSelected, &relative});
                }
            }
        }

        ImGui::End();
    }
} // namespace star::editor
