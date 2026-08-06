#include "project_manager.hpp"

#include <cstdlib>
#include <exception>
#include <fstream>

#include <nlohmann/json.hpp>

#include "star/core/logger.hpp"

namespace star::editor {
    namespace detail {
        constexpr std::size_t MAX_RECENT = 10;

        std::filesystem::path env_path(const char* name) {
            if (const char* value = std::getenv(name); value && *value)
                return std::filesystem::path{value};
            return {};
        }
    } // namespace detail

    ProjectManager::ProjectManager() {
        load_recent();
    }

    bool ProjectManager::open(const std::filesystem::path& path) {
        auto loaded = project::Project::load(path);
        if (!loaded)
            return false;
        m_active = std::move(loaded);
        remember(m_active->root());
        return true;
    }

    bool ProjectManager::create(const std::filesystem::path& parent_dir, const std::string_view name) {
        auto created = project::Project::create(parent_dir, name);
        if (!created)
            return false;
        m_active = std::move(created);
        remember(m_active->root());
        return true;
    }

    void ProjectManager::close() {
        m_active.reset();
    }

    void ProjectManager::remember(const std::filesystem::path& root) {
        std::erase(m_recent, root);
        m_recent.insert(m_recent.begin(), root);
        if (m_recent.size() > detail::MAX_RECENT)
            m_recent.resize(detail::MAX_RECENT);
        save_recent();
    }

    std::filesystem::path ProjectManager::config_dir() {
#ifdef _WIN32
        std::filesystem::path base = detail::env_path("LOCALAPPDATA");
#else
        std::filesystem::path base = detail::env_path("XDG_CONFIG_HOME");
        if (base.empty()) {
            if (const auto home = detail::env_path("HOME"); !home.empty())
                base = home / ".config";
        }
#endif
        if (base.empty())
            base = std::filesystem::current_path();
        return base / "StarEngine";
    }

    std::filesystem::path ProjectManager::recent_file() {
        return config_dir() / "recent_projects.json";
    }

    std::filesystem::path ProjectManager::default_projects_dir() {
#ifdef _WIN32
        if (const auto home = detail::env_path("USERPROFILE"); !home.empty())
            return home / "Documents" / "StarProjects";
#else
        if (const auto home = detail::env_path("HOME"); !home.empty())
            return home / "StarProjects";
#endif
        return std::filesystem::current_path() / "StarProjects";
    }

    void ProjectManager::load_recent() {
        std::ifstream in(recent_file());
        if (!in)
            return;

        nlohmann::json doc;
        try {
            in >> doc;
        } catch (const std::exception&) {
            return;
        }
        if (!doc.is_array())
            return;

        for (const auto& entry : doc) {
            if (!entry.is_string())
                continue;
            std::filesystem::path root = entry.get<std::string>();
            std::error_code ec;
            if (std::filesystem::exists(root, ec))
                m_recent.push_back(std::move(root));
        }
    }

    void ProjectManager::save_recent() const {
        std::error_code ec;
        std::filesystem::create_directories(config_dir(), ec);

        std::ofstream out(recent_file());
        if (!out)
            return;

        nlohmann::json doc = nlohmann::json::array();
        for (const auto& root : m_recent)
            doc.push_back(root.generic_string());
        out << doc.dump(2);
    }
} // namespace star::editor
