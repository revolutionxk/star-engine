#pragma once
#include "components/camera.hpp"
#include "node3d.hpp"

namespace star::scene {
    class Camera3D : public Node3D {
      public:
        Camera3D(flecs::world& world, const std::string& name);
        ~Camera3D() override = default;

        void set_fov(float fov);
        void set_near_far(float near_plane, float far_plane);
        void set_aspect_ratio(float aspect);
        void make_current();

        Matrix4 view_matrix();
        [[nodiscard]] Matrix4 projection_matrix() const;

        components::Camera& camera() {
            return get_component<components::Camera>();
        }

        [[nodiscard]] const components::Camera& camera() const {
            return m_entity.get<components::Camera>();
        }

        void on_ready() override;
    };
} // namespace star::scene
