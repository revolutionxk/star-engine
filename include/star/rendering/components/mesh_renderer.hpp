#pragma once
#include "star/core/types.hpp"
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
