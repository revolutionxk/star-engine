#pragma once
#include "star/core/reflection/type_registry.hpp"
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

namespace star {
    template<>
    struct reflection::TypeInfo<components::Active> {
        static constexpr std::string_view name = "Active";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple(field("Value", &components::Active::value));
    };
} // namespace star

STAR_REGISTER_COMPONENT(star::components::Active);

namespace star {
    template<>
    struct reflection::TypeInfo<components::Disabled> {
        static constexpr std::string_view name = "Disabled";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple();
    };
} // namespace star

STAR_REGISTER_COMPONENT(star::components::Disabled);

namespace star {
    template<>
    struct reflection::TypeInfo<components::Ready> {
        static constexpr std::string_view name = "Ready";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple();
    };
} // namespace star

STAR_REGISTER_COMPONENT(star::components::Ready);
