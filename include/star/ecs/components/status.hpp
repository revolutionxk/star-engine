#pragma once
#include "node3d.hpp"
#include "star/core/meta/type_registry.hpp"
#include "star/ecs/component_registry.hpp"

namespace star::components {
    struct Active {
        bool value{true};

        explicit operator bool() const {
            return value;
        }
    };

    struct Disabled {};

    struct Ready {};

} // namespace star::components

template<>
struct meta::TypeInfo<components::Active> {
    static constexpr std::string_view name = "Active";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple(field("Value", &components::Active::value));
};

STAR_REGISTER_COMPONENT(star::components::Active);

template<>
struct meta::TypeInfo<components::Disabled> {
    static constexpr std::string_view name = "Disabled";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple();
};

STAR_REGISTER_COMPONENT(star::components::Disabled);

template<>
struct meta::TypeInfo<components::Ready> {
    static constexpr std::string_view name = "Ready";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple();
};

STAR_REGISTER_COMPONENT(star::components::Ready);
