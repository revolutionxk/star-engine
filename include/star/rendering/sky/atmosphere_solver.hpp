#pragma once
#include "star/core/types.hpp"
#include "star/math/math.hpp"
#include "star/rendering/procedural_sky.hpp"

namespace star::rendering {
    struct AtmosphereSnapshot;

    struct AtmosphericLighting {
        bool valid{false};
        Vector3 sun_direction{0.0f, 1.0f, 0.0f};
        Vector3 directional_dir{0.0f, -1.0f, 0.0f};
        Vector3 sun_color_rgb{0.0f, 0.0f, 0.0f};
        f32 sun_intensity{1.0f};
        Vector3 sky_color_rgb{0.0f, 0.0f, 0.0f};
    };

    struct AtmosphereSolution {
        SkyParams sky_params{};
        AtmosphericLighting lighting{};
    };

    [[nodiscard]] AtmosphereSolution solve_atmosphere(const AtmosphereSnapshot& snapshot, f32 elapsed_time);
} // namespace star::rendering
