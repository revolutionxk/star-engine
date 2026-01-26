#pragma once

namespace star::components {
    struct Camera {
        float fov_y = 45.0f;
        float near_plane = 0.1f;
        float far_plane = 1000.0f;

        float aspect_ratio = 16.0f / 9.0f;

        bool is_primary = true;
    };
} // namespace star::components
