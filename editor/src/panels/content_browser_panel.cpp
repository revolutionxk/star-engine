#include "content_browser_panel.hpp"

#include <algorithm>
#include <cctype>
#include <string>

#include <imgui.h>

#include "../core/editor_events.hpp"
#include "../core/icon_registry.hpp"
#include "../editor_window.hpp"
#include "star/project/project.hpp"
#include "star/scene/scene.hpp"

namespace star::editor {
    struct Visual {
        const char* icon;
        ImVec4 tint;
    };

    std::string lower(std::string s) {
        std::ranges::transform(s, s.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    Visual visual_for(const std::filesystem::directory_entry& entry, const bool is_dir) {
        if (is_dir)
            return {"folder", ImVec4(0.86f, 0.73f, 0.42f, 1.0f)};

        const std::string name = entry.path().filename().string();
        const std::string ext = lower(entry.path().extension().string());

        if (name.ends_with(project::layout::SCENE_EXTENSION))
            return {"clapperboard", ImVec4(0.45f, 0.72f, 1.0f, 1.0f)};
        if (name.ends_with(project::layout::MATERIAL_EXTENSION))
            return {"palette", ImVec4(1.0f, 0.68f, 0.35f, 1.0f)};
        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".tga" || ext == ".bmp")
            return {"image", ImVec4(0.55f, 0.85f, 0.55f, 1.0f)};
        if (ext == ".hdr" || ext == ".exr")
            return {"image", ImVec4(0.95f, 0.6f, 0.9f, 1.0f)};
        if (ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".fbx")
            return {"box", ImVec4(0.72f, 0.6f, 1.0f, 1.0f)};
        if (ext == ".json" || ext == ".txt")
            return {"file-text", ImVec4(0.82f, 0.82f, 0.85f, 1.0f)};
        return {"file", ImVec4(0.72f, 0.72f, 0.75f, 1.0f)};
    }

    bool passes_filter(const std::string& name, const char* filter) {
        if (!filter || filter[0] == '\0')
            return true;
        return lower(name).find(lower(filter)) != std::string::npos;
    }

    ContentBrowserPanel::ContentBrowserPanel(EditorWindow* editor_window)
        : Panel("Content Browser"), m_editor_window(editor_window), m_root(std::filesystem::current_path()),
          m_current(std::filesystem::current_path()) {}

    ContentBrowserPanel::~ContentBrowserPanel() = default;

    void ContentBrowserPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        if (!m_icons)
            m_icons = &m_editor_window->icons();

        if (const auto* project = m_editor_window->projects().active(); project && m_root != project->assets_dir()) {
            m_root = project->assets_dir();
            m_current = m_root;
            m_selected.clear();
        }

        ImGui::Begin(m_name.c_str(), &m_is_open);

        if (!m_editor_window->has_project()) {
            ImGui::TextDisabled("No project open.");
            ImGui::End();
            return;
        }

        std::error_code ec;
        if (!std::filesystem::is_directory(m_current, ec)) {
            m_current = m_root;
            ImGui::End();
            return;
        }

        draw_toolbar();
        ImGui::Separator();

        std::vector<std::filesystem::directory_entry> dirs;
        std::vector<std::filesystem::directory_entry> files;
        for (const auto& entry : std::filesystem::directory_iterator(m_current, ec)) {
            if (!passes_filter(entry.path().filename().string(), m_search.data()))
                continue;
            if (entry.is_directory(ec))
                dirs.push_back(entry);
            else
                files.push_back(entry);
        }
        const auto by_name = [](const auto& a, const auto& b) { return a.path().filename() < b.path().filename(); };
        std::ranges::sort(dirs, by_name);
        std::ranges::sort(files, by_name);

        ImGui::BeginChild("##content", ImVec2(0, 0), false);
        if (m_view_mode == ViewMode::Grid)
            draw_grid(dirs, files);
        else
            draw_list(dirs, files);
        ImGui::EndChild();

        ImGui::End();
    }

    void ContentBrowserPanel::draw_toolbar() {
        const float h = ImGui::GetFrameHeight();
        const ImVec2 btn{h, h};

        ImGui::BeginDisabled(m_current == m_root);
        if (ImGui::ImageButton("##back", m_icons->icon("arrow-left"), btn, {0, 0}, {1, 1}, {0, 0, 0, 0},
                               {0.85f, 0.85f, 0.9f, 1.0f})) {
            m_current = m_current.parent_path();
            m_selected.clear();
        }
        ImGui::EndDisabled();

        ImGui::SameLine();
        const std::string crumb = m_current.lexically_relative(m_root).string();
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("Assets%s%s", crumb == "." ? "" : "/", crumb == "." ? "" : crumb.c_str());

        constexpr float right_w = 340.0f;
        ImGui::SameLine(ImGui::GetContentRegionMax().x - right_w);

        ImGui::SetNextItemWidth(150.0f);
        ImGui::InputTextWithHint("##search", "Search...", m_search.data(), m_search.size());

        ImGui::SameLine();
        const auto toggle = [&](const char* icon, const ViewMode mode, const char* tip) {
            const bool active = m_view_mode == mode;
            const ImVec4 tint = active ? ImVec4(0.45f, 0.72f, 1.0f, 1.0f) : ImVec4(0.7f, 0.7f, 0.75f, 1.0f);
            if (ImGui::ImageButton(icon, m_icons->icon(icon), btn, {0, 0}, {1, 1}, {0, 0, 0, 0}, tint))
                m_view_mode = mode;
            ImGui::SetItemTooltip("%s", tip);
        };
        toggle("layout-grid", ViewMode::Grid, "Grid view (large icons)");
        ImGui::SameLine();
        toggle("list", ViewMode::List, "List view");

        if (m_view_mode == ViewMode::Grid) {
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90.0f);
            ImGui::SliderFloat("##size", &m_thumb_size, 48.0f, 160.0f, "");
            ImGui::SetItemTooltip("Thumbnail size");
        }
    }

    void ContentBrowserPanel::draw_grid(const std::vector<std::filesystem::directory_entry>& dirs,
                                        const std::vector<std::filesystem::directory_entry>& files) {
        const ImGuiStyle& style = ImGui::GetStyle();
        const float cell = m_thumb_size + style.ItemSpacing.x;
        const float avail = ImGui::GetContentRegionAvail().x;
        const int columns = std::max(1, static_cast<int>(avail / cell));

        int index = 0;
        const auto draw_cell = [&](const std::filesystem::directory_entry& entry, const bool is_dir) {
            const std::string name = entry.path().filename().string();
            const Visual v = visual_for(entry, is_dir);
            const bool selected = m_selected == entry.path();

            ImGui::PushID(name.c_str());
            ImGui::BeginGroup();

            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.34f, 0.5f, 0.6f));
            else
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.08f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1, 1, 1, 0.14f));

            if (ImGui::ImageButton("##thumb", m_icons->icon(v.icon), ImVec2(m_thumb_size, m_thumb_size), {0, 0}, {1, 1},
                                   {0, 0, 0, 0}, v.tint))
                select(entry);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                activate(entry, is_dir);

            ImGui::PopStyleColor(3);

            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + m_thumb_size);
            ImGui::TextWrapped("%s", name.c_str());
            ImGui::PopTextWrapPos();

            ImGui::EndGroup();
            ImGui::PopID();

            if (++index % columns != 0)
                ImGui::SameLine(0.0f, style.ItemSpacing.x);
            else
                ImGui::Dummy(ImVec2(0, style.ItemSpacing.y));
        };

        for (const auto& d : dirs)
            draw_cell(d, true);
        for (const auto& f : files)
            draw_cell(f, false);
    }

    void ContentBrowserPanel::draw_list(const std::vector<std::filesystem::directory_entry>& dirs,
                                        const std::vector<std::filesystem::directory_entry>& files) {
        const float h = ImGui::GetTextLineHeight() + 4.0f;

        const auto draw_row = [&](const std::filesystem::directory_entry& entry, const bool is_dir) {
            const std::string name = entry.path().filename().string();
            const Visual v = visual_for(entry, is_dir);
            const bool selected = m_selected == entry.path();

            ImGui::PushID(name.c_str());
            const ImVec2 pos = ImGui::GetCursorScreenPos();
            if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, h)))
                select(entry);
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                activate(entry, is_dir);

            ImDrawList* dl = ImGui::GetWindowDrawList();
            constexpr float pad = 2.0f;
            dl->AddImage(m_icons->icon(v.icon), ImVec2(pos.x + pad, pos.y + pad),
                         ImVec2(pos.x + h - pad, pos.y + h - pad), ImVec2(0, 0), ImVec2(1, 1),
                         ImGui::ColorConvertFloat4ToU32(v.tint));
            dl->AddText(ImVec2(pos.x + h + 4.0f, pos.y + (h - ImGui::GetTextLineHeight()) * 0.5f),
                        ImGui::GetColorU32(ImGuiCol_Text), name.c_str());
            ImGui::PopID();
        };

        for (const auto& d : dirs)
            draw_row(d, true);
        for (const auto& f : files)
            draw_row(f, false);
    }

    void ContentBrowserPanel::select(const std::filesystem::directory_entry& entry) {
        m_selected = entry.path();

        const std::string name = entry.path().filename().string();
        if (name.ends_with(project::layout::MATERIAL_EXTENSION)) {
            if (const auto* project = m_editor_window->projects().active()) {
                std::string relative = project->to_relative(entry.path()).generic_string();
                EditorEventBus::instance().publish({EditorEventType::MaterialAssetSelected, &relative});
            }
        }
    }

    void ContentBrowserPanel::activate(const std::filesystem::directory_entry& entry, const bool is_dir) {
        if (is_dir) {
            m_current = entry.path();
            m_selected.clear();
            return;
        }
        const std::string name = entry.path().filename().string();
        if (name.ends_with(project::layout::SCENE_EXTENSION))
            (void)m_editor_window->open_scene_file(entry.path());
    }
} // namespace star::editor
