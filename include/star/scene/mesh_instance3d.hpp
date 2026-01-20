#pragma once
#include "node3d.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/rendering/components/material.hpp"
#include "star/rendering/components/mesh_renderer.hpp"

namespace star::resources {
    struct Mesh;
}

namespace star::scene {
    class MeshInstance3D : public Node3D {
      public:
        MeshInstance3D(flecs::world& world, const std::string& name);
        ~MeshInstance3D() override = default;

        void set_mesh(graphics::ResourceHandle<resources::Mesh> mesh);
        void set_material(graphics::ResourceHandle<components::Material> material);
        void set_visible(bool visible);
        void set_layer(u8 layer);

        components::MeshRenderer& renderer() {
            return get_component<components::MeshRenderer>();
        }

        void on_ready() override;
    };
} // namespace star::scene
