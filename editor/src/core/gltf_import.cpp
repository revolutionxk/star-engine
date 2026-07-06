#include "gltf_import.hpp"

#include "star/core/logger.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/import/gltf_importer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/scene/entity.hpp"
#include "star/scene/scene.hpp"

#ifdef _WIN32
    #undef NOGDI
    #include <windows.h>
    #ifndef _WINGDI_
        #include <wingdi.h>
    #endif
    #include <commdlg.h>
    #pragma comment(lib, "comdlg32.lib")
#endif

namespace star::editor {
    // SORRY FOR THIS TRASH CODE ;( IS 2:00 AM

    namespace {
        std::filesystem::path open_file_dialog(const wchar_t* title, const wchar_t* filter) {
#ifdef _WIN32
            wchar_t file[MAX_PATH] = {};
            OPENFILENAMEW ofn = {};
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = nullptr;
            ofn.lpstrFilter = filter;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrTitle = title;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
            if (GetOpenFileNameW(&ofn))
                return std::filesystem::path{file};
            return {};
#else
            (void)title;
            (void)filter;
            STAR_LOG_WARN(LogCategory::Editor, "Native file dialog not implemented on this platform");
            return {};
#endif
        }
    } // namespace

    std::filesystem::path open_model_file_dialog() {
        return open_file_dialog(L"Import glTF model", L"glTF models\0*.gltf;*.glb\0All files\0*.*\0");
    }

    std::filesystem::path open_environment_file_dialog() {
        return open_file_dialog(L"Load HDR environment", L"HDR images\0*.hdr\0All files\0*.*\0");
    }

    namespace {
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
    } // namespace

    u64 import_gltf_into_scene(const scene::Scene& scene, resources::ResourceManager& resources,
                               const std::filesystem::path& path) {
        const resources::ImportedModel model = resources::import_gltf(resources, path);
        if (!model.valid()) {
            STAR_LOG_ERROR(LogCategory::Editor, "glTF import produced no geometry: {}", path.string());
            return 0;
        }

        scene::Entity container = scene.create_entity(model.name);
        const scene::Entity root{scene.root()};
        container.child_of(root);
        for (const u32 r : model.roots)
            spawn_node(scene, model, r, container);

        STAR_LOG_INFO(LogCategory::Editor, "Instantiated glTF '{}' ({} nodes)", model.name, model.nodes.size());
        return container.id();
    }
} // namespace star::editor
