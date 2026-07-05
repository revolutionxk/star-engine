#pragma once

#include <filesystem>
#include <optional>
#include <string_view>
#include <vector>

#include "star/project/project.hpp"

namespace star::editor {
    class ProjectManager {
      public:
        ProjectManager();

        [[nodiscard]] project::Project* active() {
            return m_active ? &*m_active : nullptr;
        }

        [[nodiscard]] const project::Project* active() const {
            return m_active ? &*m_active : nullptr;
        }

        [[nodiscard]] bool has_active() const {
            return m_active.has_value();
        }

        bool open(const std::filesystem::path& path);
        bool create(const std::filesystem::path& parent_dir, std::string_view name);
        void close();

        [[nodiscard]] const std::vector<std::filesystem::path>& recent() const {
            return m_recent;
        }
        
        [[nodiscard]] static std::filesystem::path default_projects_dir();

      private:
        void remember(const std::filesystem::path& root);
        void load_recent();
        void save_recent() const;
        static std::filesystem::path config_dir();
        static std::filesystem::path recent_file();

        std::optional<project::Project> m_active;
        std::vector<std::filesystem::path> m_recent;
    };
} // namespace star::editor
