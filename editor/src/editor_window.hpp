#pragma once

#include <filesystem>
#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

#include "core/project_manager.hpp"
#include "star/application/app_window.hpp"
#include "star/physics/physics_system.hpp"

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::editor {
    enum class PlayState {
        Editing,
        Playing,
        Paused,
    };

    class EditorWindow : public application::AppWindow {
      public:
        explicit EditorWindow();
        ~EditorWindow() override = default;

        void play();
        void pause();
        void stop();

        [[nodiscard]] PlayState play_state() const {
            return m_play_state;
        }

        [[nodiscard]] ProjectManager& projects() {
            return m_projects;
        }

        [[nodiscard]] bool has_project() const {
            return m_projects.has_active();
        }

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

        physics::PhysicsSystem m_physics;
        PlayState m_play_state = PlayState::Editing;
        nlohmann::json m_scene_snapshot;

        ProjectManager m_projects;
        std::string m_active_scene_path;
    };
} // namespace star::editor
