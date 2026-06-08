#pragma once
#include <array>

#include "star/core/types.hpp"
#include "star/math/math.hpp"
#include "star/rendering/shader_uniforms.hpp"

namespace star::rendering {
    struct PackedLights {
        std::array<Vector4, uniforms::MAX_LIGHTS> pos_type{};
        std::array<Vector4, uniforms::MAX_LIGHTS> dir_range{};
        std::array<Vector4, uniforms::MAX_LIGHTS> color_int{};
        std::array<Vector4, uniforms::MAX_LIGHTS> cone{};
        u32 count{0};
    };

    struct LightEnvironment {
        PackedLights lights{};

        Color3 ambient_color{0.1f, 0.1f, 0.1f};
        f32 ambient_intensity{1.0f};
        Color3 ground_color{0.05f, 0.05f, 0.05f};
        f32 exposure{1.0f};
        bool has_sky{false};
        bool has_sun{false};
        Vector3 sun_direction{0.0f, 1.0f, 0.0f};
        Color3 sun_luminance_rgb{1.0f, 1.0f, 1.0f};
        Color3 sky_luminance_rgb{0.3f, 0.3f, 0.4f};
    };
} // namespace star::rendering
