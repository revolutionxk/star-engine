#pragma once
#include "flecs.h"

namespace star::components {
    struct CameraController {
        enum class Type {
            FREE_CAM,
            ORBITAL,
            ATTACHED
        };

        Type type{Type::FREE_CAM};
        bool enabled{true};

        flecs::entity target;
        float movement_speed{5.0f};
        float mouse_sensitivity{0.1f};
        float zoom_speed{2.0f};
        float min_zoom{1.0f};
        float max_zoom{45.0f};
    };
} // namespace star::components
