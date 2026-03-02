#pragma once
#include "star/core/types.hpp"
#include "star/ecs/component_registry.hpp"
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

template<>
struct reflection::TypeInfo<components::Atmosphere> {
    static constexpr std::string_view name = "Atmosphere";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple(
        field("Turbidity", &components::Atmosphere::turbidity) | attr::Speed{0.05f} | attr::Range{1.f, 10.f},
        field("Sun Elevation", &components::Atmosphere::sun_elevation) | attr::Speed{0.01f} |
            attr::Range{-1.5708f, 1.5708f},
        field("Sun Azimuth", &components::Atmosphere::sun_azimuth) | attr::Speed{0.01f} |
            attr::Range{-3.14159f, 3.14159f},
        field("Sun Intensity", &components::Atmosphere::sun_intensity) | attr::Speed{0.05f} | attr::Range{0.f, 10.f},
        field("Zenith xyY", &components::Atmosphere::zenith_xyY) | attr::Speed{0.001f},
        field("Enabled", &components::Atmosphere::enabled));
};

STAR_REGISTER_COMPONENT(star::components::Atmosphere);
