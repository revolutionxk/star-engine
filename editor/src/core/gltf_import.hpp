#pragma once

#include <filesystem>

#include "star/core/types.hpp"

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::editor {
    std::filesystem::path open_model_file_dialog();
    std::filesystem::path open_environment_file_dialog();
    u64 import_gltf_into_scene(const scene::Scene& scene, resources::ResourceManager& resources, const std::filesystem::path& path);
} // namespace star::editor
