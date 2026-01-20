#include "star/scene/node.hpp"

#include "star/core/common.hpp"

namespace star::scene {
    Node::Node(flecs::world& world, const std::string& name)
        : m_world(world), m_entity(world.entity(name.c_str())), m_name(name) {
        STAR_LOG_DEBUG(LogCategory::Scene, "Node '{}' created (entity: {})", m_name, m_entity.id());
    }

    void Node::add_child(Node* child) {
        if (!child) {
            STAR_LOG_WARN(LogCategory::Scene, "Attempted to add null child to node '{}'", m_name);
            return;
        }

        if (child == this) {
            STAR_LOG_ERROR(LogCategory::Scene, "Cannot add node '{}' as child of itself", m_name);
            return;
        }

        if (child->m_parent) {
            child->m_parent->remove_child(child);
        }

        m_children.push_back(child);
        child->m_parent = this;

        // i love that in ecs
        child->m_entity.add(flecs::ChildOf, m_entity);

        STAR_LOG_DEBUG(LogCategory::Scene, "Node '{}' added as child of '{}'", child->m_name, m_name);
    }

    void Node::remove_child(Node* child) {
        if (!child)
            return;

        if (const auto it = std::ranges::find(m_children, child); it != m_children.end()) {
            m_children.erase(it);
            child->m_parent = nullptr;

            child->m_entity.remove(flecs::ChildOf, m_entity);

            STAR_LOG_DEBUG(LogCategory::Scene, "Node '{}' removed from parent '{}'", child->m_name, m_name);
        }
    }

    Node* Node::find_child(const std::string& name) const {
        for (Node* child : m_children) {
            if (child->m_name == name) {
                return child;
            }
        }
        return nullptr;
    }
} // namespace star::scene
