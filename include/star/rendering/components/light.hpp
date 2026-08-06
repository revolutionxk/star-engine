#pragma once
#include "star/core/types.hpp"
#include "star/ecs/component_registry.hpp"
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
        Color4 color{1.0f, 1.0f, 1.0f, 1.0f};
        f32 intensity{1.0f};
        f32 ambient_intensity{0.03f};

        f32 range{10.0f};

        f32 inner_cone_angle_deg{30.0f};
        f32 outer_cone_angle_deg{45.0f};

        bool cast_shadows{false};
    };
} // namespace star::components

namespace star {
    template<>
    struct reflection::TypeInfo<components::Light> {
        static constexpr std::string_view name = "Light";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple(
            field("Type", &components::Light::type) | attr::EnumOptions{"Directional", "Point", "Spot"},
            field("Direction", &components::Light::direction) | attr::Speed{0.01f} | attr::Normalized{},
            field("Color", &components::Light::color) | attr::Color{},
            field("Intensity", &components::Light::intensity) | attr::Speed{0.01f} | attr::Range{0.f, 100.f},
            field("Ambient", &components::Light::ambient_intensity) | attr::Speed{0.001f} | attr::Range{0.f, 1.f},
            field("Range", &components::Light::range) | attr::Speed{0.05f} | attr::Range{0.1f, 1000.f},
            field("InnerAngleDeg", &components::Light::inner_cone_angle_deg) | attr::Speed{0.25f} | attr::Range{0.f, 89.f},
            field("OuterAngleDeg", &components::Light::outer_cone_angle_deg) | attr::Speed{0.25f} | attr::Range{0.1f, 89.9f},
            field("Shadows", &components::Light::cast_shadows));
    };
} // namespace star

STAR_REGISTER_COMPONENT(star::components::Light);
