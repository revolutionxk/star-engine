#pragma once
#include "star/math/math.hpp"

namespace star::components {
    struct TransformInterpolation {
        Vector3 previous_position{0.0f, 0.0f, 0.0f};
        Quaternion previous_rotation{0.0f, 0.0f, 0.0f, 1.0f};
        Vector3 previous_scale{1.0f, 1.0f, 1.0f};
    };
} // namespace star::components
