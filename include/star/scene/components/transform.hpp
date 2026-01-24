#pragma once

namespace star::scene {
    struct Transform {
        Vector3 position{0.0f, 0.0f, 0.0f};
        Quaternion rotation{0.0f, 0.0f, 0.0f, 1.0f};
        Vector3 scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] Matrix4 to_matrix() const {
            const auto translation_matrix = Matrix4::translate(position);
            const auto rotation_matrix = Matrix4::from_quaternion(rotation);
            const auto scale_matrix = Matrix4::scale(scale);
            return translation_matrix * rotation_matrix * scale_matrix;
        }
    };
} // namespace star::scene
