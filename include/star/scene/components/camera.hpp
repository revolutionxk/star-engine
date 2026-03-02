#pragma once
#include "star/ecs/component_registry.hpp"

namespace star::components {
    struct Camera {
        float fov_y = 45.0f;
        float near_plane = 0.1f;
        float far_plane = 1000.0f;
        float aspect_ratio = 16.0f / 9.0f;
    };

    struct PrimaryCamera {};
} // namespace star::components

template<>
struct reflection::TypeInfo<components::Camera> {
    static constexpr std::string_view name = "Camera";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple(
        field("FOV (Y)", &components::Camera::fov_y) | attr::Speed{0.5f} | attr::Range{1.f, 179.f},
        field("Near Plane", &components::Camera::near_plane) | attr::Speed{0.01f} | attr::Range{0.001f, 10.f},
        field("Far Plane", &components::Camera::far_plane) | attr::Speed{1.f} | attr::Range{1.f, 100000.f},
        field("Aspect Ratio", &components::Camera::aspect_ratio) | attr::Speed{0.01f} | attr::Range{0.1f, 4.f});
};

STAR_REGISTER_COMPONENT(star::components::Camera);

template<>
struct reflection::TypeInfo<components::PrimaryCamera> {
    static constexpr std::string_view name = "PrimaryCamera";
    static constexpr bool is_component = true;
    static constexpr bool hidden = true;
    static constexpr auto fields = std::make_tuple();
};

STAR_REGISTER_COMPONENT(star::components::PrimaryCamera, star::ecs::RegistrationFlags::Hidden);
