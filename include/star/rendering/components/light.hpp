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
        Vector3 color{1.0f, 1.0f, 1.0f};
        f32 intensity{1.0f};
        f32 ambient_intensity{0.03f};
        bool cast_shadows{false};
    };
} // namespace star::components

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
        field("Shadows", &components::Light::cast_shadows));
};

STAR_REGISTER_COMPONENT(star::components::Light);
