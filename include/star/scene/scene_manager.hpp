#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include "scene.hpp"

namespace star::scene {
    class SceneManager {
      public:
        SceneManager() = default;
        ~SceneManager() = default;

        Scene* create_scene(const std::string& name);
        Scene* get_scene(const std::string& name);

        Scene* get_active_scene() const {
            return m_active_scene;
        }

        bool has_scene(const std::string& name) const;
        void set_active_scene(const std::string& name);
        void destroy_scene(const std::string& name);
        void destroy_all_scenes();
        void update(float delta_time) const;

        size_t scene_count() const {
            return m_scenes.size();
        }

      private:
        std::unordered_map<std::string, std::unique_ptr<Scene>> m_scenes;
        Scene* m_active_scene{nullptr};
    };
} // namespace star::scene
