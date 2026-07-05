#pragma once

#include <optional>

#include <flecs.h>

#include "star/math/math.hpp"

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    class Viewport;
} // namespace star::rendering

namespace star::editor {
    class ScenePicker {
      public:
        static void ray_from_viewport(const rendering::Viewport& viewport, f32 u, f32 v, Vector3& out_origin,
                                      Vector3& out_direction);

        [[nodiscard]] static std::optional<flecs::entity> pick(const Vector3& origin, const Vector3& direction,
                                                               scene::Scene& scene,
                                                               resources::ResourceManager& resources);

      private:
        static Matrix4 world_matrix(flecs::entity entity);
    };
} // namespace star::editor
