#pragma once
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
