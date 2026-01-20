#pragma once
#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"

namespace star::resources {
    struct Mesh;
}

namespace star::components {
    struct Material;

    struct MeshRenderer {
        graphics::ResourceHandle<resources::Mesh> mesh{};
        graphics::ResourceHandle<Material> material{};

        bool visible{true};
        u8 layer{0};
        bool cast_shadow{true};
        bool receive_shadow{true};
    };
} // namespace star::components
