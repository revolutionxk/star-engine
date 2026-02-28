#pragma once
#include "star/core/types.hpp"
#include "star/math/math.hpp"

namespace star::components {
    struct Atmosphere {
        f32 turbidity = 2.5f;

        f32 sun_elevation = 0.4f;
        f32 sun_azimuth = 0.0f;

        f32 sun_intensity = 1.0f;

        Vector3 zenith_xyY{0.307f, 0.328f, 0.9f};

        bool enabled = true;
    };
} // namespace star::components
