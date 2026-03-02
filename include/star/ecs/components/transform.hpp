#pragma once
#include "star/ecs/component_registry.hpp"
#include "star/math/math.hpp"

namespace star::components {
    struct Transform {
        mutable Vector3 position{0.0f, 0.0f, 0.0f};
        mutable Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};
        mutable Vector3 scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] Matrix4 to_matrix() const {
            const auto translation_matrix = Matrix4::translate(position);
            const auto rotation_matrix = Matrix4::from_quaternion(rotation);
            const auto scale_matrix = Matrix4::scale(scale);
            return translation_matrix * rotation_matrix * scale_matrix;
        }

        void look_at(const Vector3& target, const Vector3& up = Vector3::up()) const {
            const Matrix4 look_at_matrix = Matrix4::look_at(position, target, up);
            rotation = look_at_matrix.to_quaternion();
        }

        Vector3 forward() const {
            return rotation * Vector3::forward();
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
