#include "gltf_import.hpp"

#include "star/core/logger.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/import/gltf_importer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/entity.hpp"
#include "star/scene/scene.hpp"

namespace star::editor {
    namespace detail {
        void spawn_node(const scene::Scene& scene, const resources::ImportedModel& model, const u32 index,
                        const scene::Entity& parent) {
            const auto& [name, position, rotation, scale, primitives, children] = model.nodes[index];

            scene::Entity entity = scene.create_entity(name);
            entity.set<components::Transform>({
                .position = position,
                .rotation = rotation,
                .scale = scale,
            });
            entity.child_of(parent);

            if (primitives.size() == 1) {
                entity.set<components::MeshRenderer>({.mesh = primitives[0].mesh, .material = primitives[0].material});
            } else {
                for (u32 p = 0; p < primitives.size(); ++p) {
                    scene::Entity prim = scene.create_entity(name + "_prim" + std::to_string(p));
                    prim.set<components::Transform>({});
                    prim.set<components::MeshRenderer>(
                        {.mesh = primitives[p].mesh, .material = primitives[p].material});
                    prim.child_of(entity);
                }
            }

            for (const u32 child : children)
                spawn_node(scene, model, child, entity);
        }
    } // namespace detail

    u64 instantiate_model(const scene::Scene& scene, const resources::ImportedModel& model) {
        if (!model.valid()) {
            STAR_LOG_ERROR(LogCategory::Editor, "glTF import produced no geometry: {}", model.name);
            return 0;
        }

        scene::Entity container = scene.create_entity(model.name);
        const scene::Entity root{scene.root()};
        container.child_of(root);
        for (const u32 r : model.roots)
            detail::spawn_node(scene, model, r, container);

        STAR_LOG_INFO(LogCategory::Editor, "Instantiated glTF '{}' ({} nodes)", model.name, model.nodes.size());
        return container.id();
    }

    u64 import_gltf_into_scene(const scene::Scene& scene, resources::ResourceManager& resources,
                               const std::filesystem::path& path) {
        return instantiate_model(scene, resources::import_gltf(resources, path));
    }
} // namespace star::editor
