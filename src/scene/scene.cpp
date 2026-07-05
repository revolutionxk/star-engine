#include "star/scene/scene.hpp"

#include "star/core/common.hpp"
#include "star/ecs/components/status.hpp"
#include "world_module.hpp"

namespace star::scene {
    Scene::Scene(const std::string& name) : m_name(name) {
        STAR_LOG_INFO(LogCategory::Scene, "Creating scene '{}'", m_name);

        auto& world = m_world.native();
        world.import <WorldModule>();
        world.import <flecs::units>();
        world.import <flecs::stats>();

        m_root = world.entity((m_name + "_Root").c_str());
    }

    Scene::~Scene() {
        shutdown();
    }

    Entity Scene::create_entity(const std::string& name) const {
        const auto entity = m_world.native().entity(name.c_str());
        entity.child_of(m_root);
        return Entity{entity};
    }

    Entity Scene::find_entity(const std::string& name) const {
        return Entity{m_world.native().lookup(name.c_str())};
    }

    Entity Scene::find_by_path(const std::string& path) const {
        return Entity{m_world.native().lookup(path.c_str())};
    }

    void Scene::ready() {
        if (m_is_ready) {
            STAR_LOG_WARN(LogCategory::Scene, "Scene '{}' is already ready", m_name);
            return;
        }

        m_world.native().each([&](const flecs::entity entity, const components::Active& active) {
            if (!active)
                return;

            entity.add<components::Ready>();
        });

        m_is_ready = true;
    }

    void Scene::update(const float dt) {
        if (!m_active)
            return;

        m_world.progress(dt);
    }

    void Scene::shutdown() {
        if (!m_is_ready)
            return;

        STAR_LOG_INFO(LogCategory::Scene, "Shutting down scene '{}'", m_name);

        m_is_ready = false;
    }
} // namespace star::scene
