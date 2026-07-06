#include "editor_window.hpp"

#include <filesystem>

#include "core/editor_events.hpp"
#include "core/icon_registry.hpp"
#include "layers/editor_ui_layer.hpp"
#include "scenes/sample_scene.hpp"
#include "star/application/application.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/scene.hpp"
#include "star/scene/scene_manager.hpp"
#include "star/scene/scene_serializer.hpp"

namespace star::editor {
    EditorWindow::EditorWindow() : AppWindow("Star Engine Editor", {}) {}

    EditorWindow::~EditorWindow() = default;

    IconRegistry& EditorWindow::icons() {
        if (!m_icons)
            m_icons = std::make_unique<IconRegistry>(resources());
        return *m_icons;
    }

    bool EditorWindow::on_initialize() {
        STAR_LOG_INFO(LogCategory::Editor, "Initializing editor window");

        auto* scene = scene_manager().create_scene("MainScene");
        scene_manager().set_active_scene(scene->name());
        build_empty_scene(*scene);

        push_overlay(std::make_unique<EditorUILayer>(this));

        if (!m_projects.recent().empty())
            open_project(m_projects.recent().front());
        else
            update_window_title();

        STAR_LOG_INFO(LogCategory::Editor, "Editor window initialized");
        return true;
    }

    void EditorWindow::on_shutdown() {
        STAR_LOG_INFO(LogCategory::Editor, "Editor window shutting down");
    }

    void EditorWindow::on_update(const f32 delta_time) {
        set_render_interpolation(m_play_state == PlayState::Playing);
    }

    void EditorWindow::on_fixed_update(const f32 fixed_dt) {
        if (m_play_state != PlayState::Playing) {
            return;
        }
        if (auto* scene = scene_manager().get_active_scene()) {
            m_physics.update(scene->world(), fixed_dt);
        }
    }

    void EditorWindow::play() {
        if (m_play_state == PlayState::Paused) {
            m_play_state = PlayState::Playing;
            return;
        }
        if (m_play_state != PlayState::Editing) {
            return;
        }

        if (auto* scene = scene_manager().get_active_scene()) {
            m_scene_snapshot = scene::serialize_scene(*scene);
            m_play_state = PlayState::Playing;
            STAR_LOG_INFO(LogCategory::Editor, "Play");
        }
    }

    void EditorWindow::pause() {
        if (m_play_state == PlayState::Playing) {
            m_play_state = PlayState::Paused;
        } else if (m_play_state == PlayState::Paused) {
            m_play_state = PlayState::Playing;
        }
    }

    void EditorWindow::stop() {
        if (m_play_state == PlayState::Editing) {
            return;
        }

        if (auto* scene = scene_manager().get_active_scene()) {
            m_physics.reset(scene->world());
            scene::load_scene(*scene, m_scene_snapshot);
        }
        m_play_state = PlayState::Editing;
        STAR_LOG_INFO(LogCategory::Editor, "Stop");
    }

    bool EditorWindow::create_project(const std::filesystem::path& parent_dir, const std::string_view name) {
        if (!m_projects.create(parent_dir, name))
            return false;

        auto* project = m_projects.active();
        auto* scene = scene_manager().get_active_scene();
        if (!project || !scene)
            return false;

        std::error_code ec;
        std::filesystem::create_directories(project->scenes_dir(), ec);
        const auto scene_path = project->scenes_dir() / ("Main" + std::string{project::layout::SCENE_EXTENSION});

        resources().set_asset_root(project->root());

        EditorEventBus::instance().publish({EditorEventType::EntityDeselected, nullptr});
        scene::load_scene(*scene, nlohmann::json::object());
        build_sample_scene(*scene, resources());
        (void)scene::save_scene_to_file(*scene, scene_path);

        project->set_default_scene(project->to_relative(scene_path).generic_string());
        project->save();
        m_active_scene_path = scene_path.string();

        update_window_title();
        EditorEventBus::instance().publish({EditorEventType::SceneLoaded, nullptr});
        return true;
    }

    bool EditorWindow::open_project(const std::filesystem::path& path) {
        if (!m_projects.open(path))
            return false;
        if (const auto* project = m_projects.active())
            resources().set_asset_root(project->root());
        load_project_scene();
        update_window_title();
        return true;
    }

    void EditorWindow::load_project_scene() {
        auto* scene = scene_manager().get_active_scene();
        auto* project = m_projects.active();
        if (!scene || !project)
            return;

        EditorEventBus::instance().publish({EditorEventType::EntityDeselected, nullptr});

        const auto scene_path = project->resolve(project->default_scene());
        std::error_code ec;
        if (!project->default_scene().empty() && std::filesystem::exists(scene_path, ec) &&
            scene::load_scene_from_file(*scene, scene_path)) {
            m_active_scene_path = scene_path.string();
            EditorEventBus::instance().publish({EditorEventType::SceneLoaded, nullptr});
            return;
        }

        scene::load_scene(*scene, nlohmann::json::object());
        build_empty_scene(*scene);
        m_active_scene_path.clear();
    }

    void EditorWindow::save_active_scene() {
        auto* scene = scene_manager().get_active_scene();
        if (!scene)
            return;

        std::filesystem::path path;
        if (!m_active_scene_path.empty()) {
            path = m_active_scene_path;
        } else if (auto* project = m_projects.active()) {
            std::error_code ec;
            std::filesystem::create_directories(project->scenes_dir(), ec);
            path = project->scenes_dir() / (scene->name() + std::string{project::layout::SCENE_EXTENSION});
        } else {
            std::error_code ec;
            std::filesystem::create_directories("scenes", ec);
            path = std::filesystem::path("scenes") / (scene->name() + std::string{project::layout::SCENE_EXTENSION});
        }

        if (!scene::save_scene_to_file(*scene, path))
            return;

        m_active_scene_path = path.string();
        if (auto* project = m_projects.active()) {
            project->set_default_scene(project->to_relative(path).generic_string());
            project->save();
        }
        update_window_title();
        EditorEventBus::instance().publish({EditorEventType::SceneSaved, nullptr});
    }

    bool EditorWindow::open_scene_file(const std::filesystem::path& path) {
        auto* scene = scene_manager().get_active_scene();
        std::error_code ec;
        if (!scene || !std::filesystem::exists(path, ec))
            return false;

        EditorEventBus::instance().publish({EditorEventType::EntityDeselected, nullptr});
        if (!scene::load_scene_from_file(*scene, path))
            return false;

        m_active_scene_path = path.string();
        if (auto* project = m_projects.active()) {
            project->set_default_scene(project->to_relative(path).generic_string());
            project->save();
        }
        update_window_title();
        EditorEventBus::instance().publish({EditorEventType::SceneLoaded, nullptr});
        return true;
    }

    void EditorWindow::new_scene() {
        auto* scene = scene_manager().get_active_scene();
        if (!scene)
            return;

        EditorEventBus::instance().publish({EditorEventType::EntityDeselected, nullptr});
        scene::load_scene(*scene, nlohmann::json::object());
        build_empty_scene(*scene);
        m_active_scene_path.clear();
        update_window_title();
        STAR_LOG_INFO(LogCategory::Editor, "Created a new scene");
    }

    void EditorWindow::update_window_title() {
        std::string title = "Star Engine Editor";
        if (const auto* project = m_projects.active())
            title = project->name() + "  —  Star Engine";

        if (const auto* scene = scene_manager().get_active_scene()) {
            const std::string scene_label = m_active_scene_path.empty()
                                                ? scene->name()
                                                : std::filesystem::path(m_active_scene_path).stem().string();
            title += "  —  " + scene_label;
        }

        window().set_title(title);
    }
} // namespace star::editor
