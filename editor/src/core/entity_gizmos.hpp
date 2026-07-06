#pragma once

#include <optional>

#include <flecs.h>

#include "star/math/math.hpp"

namespace star::scene {
    class Scene;
} // namespace star::scene

namespace star::rendering {
    class DebugRenderer;
} // namespace star::rendering

namespace star::editor {
    class EntityGizmos {
      public:
        static void draw(scene::Scene& scene, rendering::DebugRenderer& debug,
                         const std::optional<flecs::entity>& selected);

        [[nodiscard]] static Vector3 world_position(flecs::entity entity);

      private:
        static void draw_camera(flecs::entity entity, rendering::DebugRenderer& debug, bool selected);
        static void draw_light(flecs::entity entity, rendering::DebugRenderer& debug, bool selected);
    };
} // namespace star::editor
