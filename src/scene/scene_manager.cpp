#include "star/scene/scene_manager.hpp"

#include "star/core/common.hpp"

namespace star::scene {
    Scene* SceneManager::create_scene(const std::string& name) {
        if (m_scenes.contains(name)) {
            STAR_LOG_WARN(LogCategory::Scene, "Scene '{}' already exists", name);
            return m_scenes[name].get();
        }

        STAR_LOG_INFO(LogCategory::Scene, "SceneManager: Creating scene '{}'", name);

        auto scene = std::make_unique<Scene>(name);
        Scene* ptr = scene.get();
        m_scenes[name] = std::move(scene);

        if (!m_active_scene) {
            m_active_scene = ptr;
            m_active_scene->set_active(true);
            STAR_LOG_INFO(LogCategory::Scene, "Scene '{}' set as active (first scene)", name);
        }

        return ptr;
    }

    Scene* SceneManager::get_scene(const std::string& name) {
        if (const auto it = m_scenes.find(name); it != m_scenes.end()) {
            return it->second.get();
        }
        return nullptr;
    }

    bool SceneManager::has_scene(const std::string& name) const {
        return m_scenes.contains(name);
    }

    void SceneManager::set_active_scene(const std::string& name) {
        const auto it = m_scenes.find(name);
        if (it == m_scenes.end()) {
            STAR_LOG_ERROR(LogCategory::Scene, "Cannot set active scene: '{}' not found", name);
            return;
        }

        if (m_active_scene) {
            m_active_scene->set_active(false);
            STAR_LOG_DEBUG(LogCategory::Scene, "Scene '{}' deactivated", m_active_scene->name());
        }

        m_active_scene = it->second.get();
        m_active_scene->set_active(true);

        STAR_LOG_INFO(LogCategory::Scene, "Scene '{}' is now active", name);
    }

    void SceneManager::destroy_scene(const std::string& name) {
        const auto it = m_scenes.find(name);
        if (it == m_scenes.end()) {
            STAR_LOG_WARN(LogCategory::Scene, "Cannot destroy scene: '{}' not found", name);
            return;
        }

        STAR_LOG_INFO(LogCategory::Scene, "Destroying scene '{}'", name);

        if (m_active_scene == it->second.get()) {
            m_active_scene = nullptr;
        }

        m_scenes.erase(it);
    }

    void SceneManager::destroy_all_scenes() {
        STAR_LOG_INFO(LogCategory::Scene, "Destroying all scenes");

        m_active_scene = nullptr;
        m_scenes.clear();
    }

    void SceneManager::update(const float delta_time) const {
        if (m_active_scene && m_active_scene->is_active()) {
            m_active_scene->update(delta_time);
        }
    }
} // namespace star::scene
