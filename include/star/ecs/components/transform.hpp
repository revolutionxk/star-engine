#pragma once
#include "star/ecs/component_registry.hpp"
#include "star/math/math.hpp"

namespace star::components {
    struct Transform {
        Vector3 position{0.0f, 0.0f, 0.0f};
        Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] Matrix4 to_matrix() const {
            return Matrix4::translate(position) * Matrix4::from_quaternion(rotation) * Matrix4::scale(scale);
        }

        [[nodiscard]] Vector3 forward() const {
            return rotation * Vector3::forward();
        }

        [[nodiscard]] Vector3 right() const {
            return rotation * Vector3::right();
        }

        [[nodiscard]] Vector3 up() const {
            return rotation * Vector3::up();
        }

        void look_at(const Vector3& target, const Vector3& world_up = Vector3::up()) {
            rotation = Matrix4::look_at(position, target, world_up).to_quaternion();
        }
    };
} // namespace star::components

template<>
struct reflection::TypeInfo<components::Transform> {
    static constexpr std::string_view name = "Transform";
    static constexpr bool is_component = true;
    static constexpr bool required = true;
    static constexpr auto fields = std::make_tuple(
        field("Position", &components::Transform::position) | attr::Speed{0.1f},
        field("Rotation", &components::Transform::rotation) | attr::Speed{0.5f},
        field("Scale", &components::Transform::scale) | attr::Speed{0.05f} | attr::Range{0.001f, 1000.f});
};

STAR_REGISTER_COMPONENT(star::components::Transform);
