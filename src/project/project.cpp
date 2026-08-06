#include "star/project/project.hpp"

#include <array>
#include <exception>
#include <fstream>

#include <nlohmann/json.hpp>

#include "star/core/logger.hpp"

namespace star::project {
    namespace detail {
        constexpr std::string_view ENGINE_VERSION = "0.1.0";
        constexpr const char* MANIFEST_KEY = "star_project";

        std::filesystem::path find_manifest(const std::filesystem::path& dir) {
            std::error_code ec;
            for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
                if (entry.path().extension() == layout::PROJECT_EXTENSION)
                    return entry.path();
            }
            return {};
        }
    } // namespace detail

    std::optional<Project> Project::create(const std::filesystem::path& parent_dir, const std::string_view name) {
        if (name.empty()) {
            STAR_LOG_ERROR(LogCategory::Editor, "Cannot create a project with an empty name");
            return std::nullopt;
        }

        const std::filesystem::path root = parent_dir / name;

        std::error_code ec;
        if (std::filesystem::exists(root, ec)) {
            STAR_LOG_ERROR(LogCategory::Editor, "Project directory already exists: {}", root.string());
            return std::nullopt;
        }

        Project project;
        project.m_root = root;
        project.m_name = std::string{name};
        project.m_engine_version = std::string{detail::ENGINE_VERSION};
        project.m_default_scene = std::string{layout::SCENES} + "/Main" + std::string{layout::SCENE_EXTENSION};

        for (const std::string_view sub : {layout::ASSETS, layout::SCENES, layout::CONFIG, layout::CACHE}) {
            std::filesystem::create_directories(root / sub, ec);
            if (ec) {
                STAR_LOG_ERROR(LogCategory::Editor, "Failed to create '{}': {}", (root / sub).string(), ec.message());
                return std::nullopt;
            }
        }

        if (!project.save())
            return std::nullopt;

        STAR_LOG_INFO(LogCategory::Editor, "Created project '{}' at {}", project.m_name, root.string());
        return project;
    }

    std::optional<Project> Project::load(const std::filesystem::path& path) {
        std::error_code ec;
        std::filesystem::path manifest = path;
        if (std::filesystem::is_directory(path, ec))
            manifest = detail::find_manifest(path);

        if (manifest.empty() || !std::filesystem::exists(manifest, ec)) {
            STAR_LOG_ERROR(LogCategory::Editor, "No project manifest found at {}", path.string());
            return std::nullopt;
        }

        std::ifstream in(manifest);
        if (!in) {
            STAR_LOG_ERROR(LogCategory::Editor, "Failed to open project manifest: {}", manifest.string());
            return std::nullopt;
        }

        nlohmann::json doc;
        try {
            in >> doc;
        } catch (const std::exception& e) {
            STAR_LOG_ERROR(LogCategory::Editor, "Malformed project manifest '{}': {}", manifest.string(), e.what());
            return std::nullopt;
        }

        const nlohmann::json& body = doc.contains(detail::MANIFEST_KEY) ? doc[detail::MANIFEST_KEY] : doc;

        Project project;
        project.m_root = manifest.parent_path();
        project.m_name = body.value("name", manifest.stem().string());
        project.m_engine_version = body.value("engine_version", std::string{detail::ENGINE_VERSION});
        project.m_default_scene = body.value("default_scene", std::string{});

        STAR_LOG_INFO(LogCategory::Editor, "Loaded project '{}' from {}", project.m_name, project.m_root.string());
        return project;
    }

    bool Project::save() const {
        nlohmann::json doc;
        doc[detail::MANIFEST_KEY] = {
            {"name", m_name},
            {"engine_version", m_engine_version},
            {"default_scene", m_default_scene},
        };

        std::error_code ec;
        std::filesystem::create_directories(m_root, ec);

        std::ofstream out(manifest_path());
        if (!out) {
            STAR_LOG_ERROR(LogCategory::Editor, "Failed to write project manifest: {}", manifest_path().string());
            return false;
        }
        out << doc.dump(4);
        return true;
    }

    std::filesystem::path Project::manifest_path() const {
        return m_root / (m_name + std::string{layout::PROJECT_EXTENSION});
    }

    std::filesystem::path Project::resolve(const std::filesystem::path& relative) const {
        if (relative.is_absolute())
            return relative;
        return m_root / relative;
    }

    std::filesystem::path Project::to_relative(const std::filesystem::path& absolute) const {
        std::error_code ec;
        const std::filesystem::path rel = std::filesystem::relative(absolute, m_root, ec);
        if (ec || rel.empty() || *rel.begin() == "..")
            return absolute;
        return rel;
    }
} // namespace star::project
