#include "star/scene/node3d.hpp"

#include "star/core/common.hpp"

namespace star::scene {
    Node3D::Node3D(flecs::world& world, const std::string& name) : Node(world, name) {
        add_component<Transform>(Transform{});
    }

    void Node3D::on_ready() {
        Node::on_ready();
        STAR_LOG_DEBUG(LogCategory::Scene, "Node3D '{}' ready", m_name);
    }

    void Node3D::set_position(const Vector3& pos) {
        auto& t = transform();
        t.position = pos;
    }

    void Node3D::set_rotation(const Quaternion& rot) {
        auto& t = transform();
        t.rotation = rot;
    }

    void Node3D::set_scale(const Vector3& scale) {
        auto& t = transform();
        t.scale = scale;
    }

    Vector3 Node3D::position() {
        const auto& t = transform();
        return t.position;
    }

    Quaternion Node3D::rotation() {
        const auto& t = transform();
        return t.rotation;
    }

    Vector3 Node3D::scale() {
        const auto& t = transform();
        return t.scale;
    }

    Vector3 Node3D::global_position() {
        Vector3 pos = position();

        Node* parent = get_parent();
        while (parent) {
            if (auto* parent_node3d = dynamic_cast<Node3D*>(parent)) {
                pos = pos + parent_node3d->position();
            }
            parent = parent->get_parent();
        }

        return pos;
    }

    Matrix4 Node3D::global_transform() {
        const auto& t = transform();

        const auto local_matrix = t.to_matrix();

        if (Node* parent = get_parent()) {
            if (auto* parent_node3d = dynamic_cast<Node3D*>(parent)) {
                const auto parent_matrix = parent_node3d->global_transform();
                return parent_matrix * local_matrix;
            }
        }

        return local_matrix;
    }
} // namespace star::scene
