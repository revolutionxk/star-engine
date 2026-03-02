#pragma once
#include "star/ecs/component_registry.hpp"

namespace star::components {
    struct Tag {
        std::string value;
    };
} // namespace star::components

template<>
struct reflection::TypeInfo<components::Tag> {
    static constexpr std::string_view name = "Tag";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple(field("Value", &components::Tag::value));
};

STAR_REGISTER_COMPONENT(star::components::Tag);
