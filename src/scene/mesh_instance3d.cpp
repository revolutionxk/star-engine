#include "star/scene/mesh_instance3d.hpp"

#include "star/core/common.hpp"

namespace star::scene {
    MeshInstance3D::MeshInstance3D(flecs::world& world, const std::string& name) : Node3D(world, name) {
        add_component<components::MeshRenderer>(components::MeshRenderer{});
    }

    void MeshInstance3D::on_ready() {
        Node3D::on_ready();
        STAR_LOG_DEBUG(LogCategory::Scene, "MeshInstance3D '{}' ready", m_name);
    }

    void MeshInstance3D::set_mesh(const graphics::ResourceHandle<resources::Mesh> mesh) {
        auto& mr = get_component<components::MeshRenderer>();
        mr.mesh = mesh;
    }

    void MeshInstance3D::set_material(const graphics::ResourceHandle<components::Material> material) {
        auto& mr = get_component<components::MeshRenderer>();
        mr.material = material;
    }

    void MeshInstance3D::set_visible(const bool visible) {
        auto& mr = get_component<components::MeshRenderer>();
        mr.visible = visible;
    }

    void MeshInstance3D::set_layer(const u8 layer) {
        auto& mr = get_component<components::MeshRenderer>();
        mr.layer = layer;
    }
} // namespace star::scene
