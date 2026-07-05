#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

namespace star::project {
    namespace layout {
        inline constexpr std::string_view PROJECT_EXTENSION = ".starproject";
        inline constexpr std::string_view SCENE_EXTENSION = ".starscene";
        inline constexpr std::string_view MATERIAL_EXTENSION = ".starmat";
        inline constexpr std::string_view ASSETS = "Assets";
        inline constexpr std::string_view SCENES = "Assets/Scenes";
        inline constexpr std::string_view MATERIALS = "Assets/Materials";
        inline constexpr std::string_view CONFIG = "Config";
        inline constexpr std::string_view CACHE = "Cache";
    } // namespace layout

    class Project {
      public:
        [[nodiscard]] static std::optional<Project> create(const std::filesystem::path& parent_dir,
                                                           std::string_view name);

        [[nodiscard]] static std::optional<Project> load(const std::filesystem::path& path);
        [[nodiscard]] bool save() const;

        [[nodiscard]] const std::string& name() const {
            return m_name;
        }

        [[nodiscard]] const std::filesystem::path& root() const {
            return m_root;
        }

        [[nodiscard]] std::filesystem::path manifest_path() const;

        [[nodiscard]] std::filesystem::path assets_dir() const {
            return m_root / layout::ASSETS;
        }

        [[nodiscard]] std::filesystem::path scenes_dir() const {
            return m_root / layout::SCENES;
        }

        [[nodiscard]] std::filesystem::path config_dir() const {
            return m_root / layout::CONFIG;
        }

        [[nodiscard]] std::filesystem::path cache_dir() const {
            return m_root / layout::CACHE;
        }

        [[nodiscard]] std::filesystem::path resolve(const std::filesystem::path& relative) const;
        [[nodiscard]] std::filesystem::path to_relative(const std::filesystem::path& absolute) const;
        
        [[nodiscard]] const std::string& default_scene() const {
            return m_default_scene;
        }

        void set_default_scene(std::string relative_path) {
            m_default_scene = std::move(relative_path);
        }

      private:
        std::filesystem::path m_root;
        std::string m_name;
        std::string m_engine_version;
        std::string m_default_scene;
    };
} // namespace star::project
