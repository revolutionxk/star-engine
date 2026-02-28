#pragma once

#include "star/core/meta/type_registry.hpp"
#include "star/ecs/components.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/atmosphere.hpp"
#include "star/rendering/components/light.hpp"
#include "star/scene/components/camera.hpp"

namespace star {
    inline const meta::AutoRegister<components::Transform, components::Light, components::Camera,
                                    components::Atmosphere>
        g_engine_components;
} // namespace star
