#pragma once

#include <filesystem>

#include "star/core/types.hpp"

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::resources {
    class ResourceManager;
    struct ImportedModel;
} // namespace star::resources

namespace star::editor {
    u64 import_gltf_into_scene(const scene::Scene& scene, resources::ResourceManager& resources, const std::filesystem::path& path);
    u64 instantiate_model(const scene::Scene& scene, const resources::ImportedModel& model);
} // namespace star::editor
