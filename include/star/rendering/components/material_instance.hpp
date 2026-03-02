#pragma once
#include <vector>

#include "star/ecs/component_registry.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/rendering/material_property.hpp"

namespace star::resources {
    struct Material;
} // namespace star::resources

namespace star::components {
    struct MaterialInstance {
        graphics::ResourceHandle<resources::Material> material{};
        std::vector<rendering::MaterialProperty> parameters;
    };
} // namespace star::components

template<>
struct reflection::TypeInfo<components::MaterialInstance> {
    static constexpr std::string_view name = "MaterialInstance";
    static constexpr bool is_component = true;
    static constexpr auto fields = std::make_tuple();
};

STAR_REGISTER_COMPONENT(star::components::MaterialInstance);
