#pragma once

#include "star/core/types.hpp"

namespace star::components {
    struct Camera;
    struct Transform;
} // namespace star::components

namespace star::rendering {
    class Viewport;

    using ViewId = u32;
    inline constexpr ViewId INVALID_VIEW = 0;

    struct RenderView {
        Viewport* viewport = nullptr;
        const components::Camera* camera = nullptr;
        const components::Transform* camera_transform = nullptr;
        bool draw_debug = false;
        bool enabled = true;
    };
} // namespace star::rendering
