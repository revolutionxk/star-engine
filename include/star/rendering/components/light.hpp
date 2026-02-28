#pragma once
#include "star/core/types.hpp"
#include "star/math/math.hpp"

namespace star::components {
    struct Light {
        enum class Type {
            Directional,
            Point,
            Spot
        };

        Type type = Type::Directional;

        Vector3 direction{0.0f, -1.0f, 0.0f};

        Vector3 color{1.0f, 1.0f, 1.0f};
        f32 intensity{1.0f};

        f32 ambient_intensity{0.03f};

        bool cast_shadows{false};
    };
} // namespace star::components
