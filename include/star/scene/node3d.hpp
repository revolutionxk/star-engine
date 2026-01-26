#pragma once
#include "star/ecs/components/transform.hpp"
#include "node.hpp"
#include "star/rendering/components/material.hpp"

namespace star::scene {
    class Node3D : public Node {
      public:
        Node3D(flecs::world& world, const std::string& name);

        void set_position(const Vector3& pos);
        void set_rotation(const Quaternion& rot);
        void set_scale(const Vector3& scale);

        Vector3 position();
        Vector3 global_position();
        Matrix4 global_transform();
        Quaternion rotation();
        Vector3 scale();

        Transform& transform() {
            return get_component<Transform>();
        }

      protected:
        void on_ready() override;
    };
} // namespace star::scene
