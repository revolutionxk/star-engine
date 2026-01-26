#include "star/scene/camera3d.hpp"

#include "star/core/common.hpp"

namespace star::scene {
    Camera3D::Camera3D(flecs::world& world, const std::string& name) : Node3D(world, name) {
        add_component<components::Camera>(components::Camera{});
    }

    void Camera3D::on_ready() {
        Node3D::on_ready();
        STAR_LOG_DEBUG(LogCategory::Scene, "Camera3D '{}' ready", m_name);
    }

    void Camera3D::set_fov(const float fov) {
        auto& cam = camera();
        cam.fov_y = fov;
    }

    void Camera3D::set_near_far(const float near_plane, const float far_plane) {
        auto& cam = camera();
        cam.near_plane = near_plane;
        cam.far_plane = far_plane;
    }

    void Camera3D::set_aspect_ratio(const float aspect) {
        auto& cam = camera();
        cam.aspect_ratio = aspect;
    }

    void Camera3D::make_current() {
        auto& cam = camera();
        m_world.each([](flecs::entity, components::Camera& other_cam) { other_cam.is_primary = false; });

        cam.is_primary = true;

        STAR_LOG_INFO(LogCategory::Scene, "Camera '{}' set as primary", m_name);
    }

    Matrix4 Camera3D::view_matrix() {
        const Matrix4 camera_transform = global_transform();
        return camera_transform.inversed();
    }

    Matrix4 Camera3D::projection_matrix() const {
        const auto& cam = camera();
        return Matrix4::perspective(cam.fov_y, cam.aspect_ratio, cam.near_plane, cam.far_plane);
    }
} // namespace star::scene
