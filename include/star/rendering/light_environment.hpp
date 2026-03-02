#pragma once

namespace star::rendering {
    struct LightEnvironment {
        bool has_directional{false};
        Vector3 directional_dir{0, -1, 0};
        Color3 directional_color{1, 1, 1};
        f32 directional_intensity{1.0f};
        Color3 ambient_color{0.1f, 0.1f, 0.1f};
        f32 ambient_intensity{1.0f};
    };
} // namespace star::rendering
