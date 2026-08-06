#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

#include "core/file_dialog.hpp"
#include "core/project_manager.hpp"
#include "star/resources/import/async_model_loader.hpp"
#include "star/application/app_window.hpp"
#include "star/physics/physics_system.hpp"

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::editor {
    class IconRegistry;

    enum class PlayState {
        Editing,
        Playing,
        Paused,
    };

    class EditorWindow : public application::AppWindow {
      public:
        explicit EditorWindow();
        ~EditorWindow() override;

        void play();
        void pause();
        void stop();

        [[nodiscard]] PlayState play_state() const {
            return m_play_state;
        }

        [[nodiscard]] ProjectManager& projects() {
            return m_projects;
        }

        [[nodiscard]] IconRegistry& icons();

        [[nodiscard]] bool has_project() const {
            return m_projects.has_active();
        }

        void open_file_dialog(FileRequest request);
        void queue_model_import(const std::filesystem::path& path);
        void load_environment(const std::filesystem::path& path);

        [[nodiscard]] u32 imports_in_flight() const;
        [[nodiscard]] std::vector<std::string> imports_in_flight_names() const;

        bool create_project(const std::filesystem::path& parent_dir, std::string_view name);
        bool open_project(const std::filesystem::path& path);
        
        void save_active_scene();
        bool open_scene_file(const std::filesystem::path& path);
        void new_scene();

      protected:
        bool on_initialize() override;
        void on_shutdown() override;
        void on_update(f32 delta_time) override;
        void on_fixed_update(f32 fixed_dt) override;

      private:
        void load_project_scene();
        void update_window_title();

        EditorFileDialogs m_file_dialogs;
        std::unique_ptr<resources::AsyncModelLoader> m_model_loader;
        physics::PhysicsSystem m_physics;
        PlayState m_play_state = PlayState::Editing;
        nlohmann::json m_scene_snapshot;

        ProjectManager m_projects;
        std::string m_active_scene_path;

        std::unique_ptr<IconRegistry> m_icons;
    };
} // namespace star::editor
