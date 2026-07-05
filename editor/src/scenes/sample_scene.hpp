#pragma once

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::editor {
    void build_empty_scene(const scene::Scene& scene);
    void build_sample_scene(const scene::Scene& scene, resources::ResourceManager& resources);
} // namespace star::editor
