#pragma once
#include "star/core/types.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/graphics/resource_handle.hpp"

namespace star::resources {
    struct Mesh;
    struct Material;
} // namespace star::resources

namespace star::components {
    struct MeshRenderer {
        graphics::ResourceHandle<resources::Mesh> mesh{};
        graphics::ResourceHandle<resources::Material> material{};
        bool visible{true};
        u8 layer{0};
        bool cast_shadow{true};
        bool receive_shadow{true};
    };
} // namespace star::components

namespace star {
    template<>
    struct reflection::TypeInfo<components::MeshRenderer> {
        static constexpr std::string_view name = "MeshRenderer";
        static constexpr bool is_component = true;
        static constexpr auto fields = std::make_tuple(field("Visible", &components::MeshRenderer::visible),
                                                       field("Layer", &components::MeshRenderer::layer),
                                                       field("Cast Shadow", &components::MeshRenderer::cast_shadow),
                                                       field("Receive Shadow", &components::MeshRenderer::receive_shadow));
    };
} // namespace star

namespace star::components {
    [[nodiscard]] nlohmann::json to_json(const MeshRenderer& value);
    void from_json(const nlohmann::json& j, MeshRenderer& value);
} // namespace star::components

STAR_REGISTER_COMPONENT(star::components::MeshRenderer);
