#include "star/scene/scene.hpp"

#include "star/core/common.hpp"
#include "star/ecs/components/transform.hpp"
#include "world_module.hpp"

namespace star::scene {
    Scene::Scene(const std::string& name) : m_name(name) {
        STAR_LOG_INFO(LogCategory::Scene, "Creating scene '{}'", m_name);

        m_world.import <WorldModule>();
    }

    Scene::~Scene() {
        shutdown();
    }

    void Scene::ready() {
        if (m_is_ready) {
            STAR_LOG_WARN(LogCategory::Scene, "Scene '{}' is already ready", m_name);
            return;
        }

        STAR_LOG_INFO(LogCategory::Scene, "Scene '{}': Calling ready on all nodes", m_name);

        for (const auto& node : m_nodes) {
            node->on_ready();
        }

        m_is_ready = true;
    }

    void Scene::update(const float dt) const {
        if (!m_active)
            return;

        for (const auto& node : m_nodes) {
            node->on_update(dt);
        }

        m_world.progress(dt);
    }

    void Scene::shutdown() {
        if (!m_is_ready)
            return;

        STAR_LOG_INFO(LogCategory::Scene, "Shutting down scene '{}'", m_name);

        for (const auto& node : m_nodes) {
            node->on_exit();
        }

        m_nodes.clear();
        m_root_nodes.clear();

        m_is_ready = false;
    }

    Node* Scene::find_node(const std::string& path) const {
        if (path.empty())
            return nullptr;

        if (path.find('/') == std::string::npos) {
            return find_node_by_name(path);
        }
        Node* current = nullptr;
        size_t start = 0;
        size_t end = path.find('/');

        const std::string first_name = path.substr(0, end);
        for (Node* root : m_root_nodes) {
            if (root->name() == first_name) {
                current = root;
                break;
            }
        }

        if (!current)
            return nullptr;

        start = end + 1;
        while (start < path.length()) {
            end = path.find('/', start);
            if (end == std::string::npos) {
                end = path.length();
            }

            std::string child_name = path.substr(start, end - start);
            current = current->find_child(child_name);

            if (!current)
                return nullptr;

            start = end + 1;
        }

        return current;
    }

    Node* Scene::find_node_by_name(const std::string& name) const {
        for (auto& node : m_nodes) {
            if (node->name() == name) {
                return node.get();
            }
        }
        return nullptr;
    }
} // namespace star::scene
