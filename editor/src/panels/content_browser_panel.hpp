#pragma once

#include <array>
#include <filesystem>
#include <memory>
#include <vector>

#include "panel.hpp"

namespace star::editor {
    class EditorWindow;
    class IconRegistry;

    class ContentBrowserPanel final : public Panel {
      public:
        explicit ContentBrowserPanel(EditorWindow* editor_window);
        ~ContentBrowserPanel() override;

        void on_imgui_render() override;

      private:
        enum class ViewMode {
            Grid,
            List,
        };

        void draw_toolbar();
        void draw_grid(const std::vector<std::filesystem::directory_entry>& dirs,
                       const std::vector<std::filesystem::directory_entry>& files);
        void draw_list(const std::vector<std::filesystem::directory_entry>& dirs,
                       const std::vector<std::filesystem::directory_entry>& files);
        void activate(const std::filesystem::directory_entry& entry, bool is_dir);
        void select(const std::filesystem::directory_entry& entry);

        EditorWindow* m_editor_window;
        std::unique_ptr<IconRegistry> m_icons;

        std::filesystem::path m_root;
        std::filesystem::path m_current;
        std::filesystem::path m_selected;

        ViewMode m_view_mode{ViewMode::Grid};
        f32 m_thumb_size{84.0f};
        std::array<char, 128> m_search{};
    };
} // namespace star::editor
