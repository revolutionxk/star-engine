#pragma once
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"

namespace star::resources {
    struct Mesh;
}

namespace star::components {
    struct Material;
}

namespace star::rendering {
    struct RenderCommand {
        Matrix4 model_matrix;
        Matrix4 mvp_matrix;
        graphics::ResourceHandle<resources::Mesh> mesh;
        graphics::ResourceHandle<components::Material> material;
        u64 sort_key{0};
        f32 distance_to_camera{0.0f};
        u8 layer{0};
        bool is_transparent{false};
    };

    class RenderQueue {
      public:
        RenderQueue();
        ~RenderQueue();

        void submit(const RenderCommand& command);
        void sort();
        void clear();

        const std::vector<RenderCommand>& opaque_commands() const {
            return m_opaque_commands;
        }

        const std::vector<RenderCommand>& transparent_commands() const {
            return m_transparent_commands;
        }

        size_t command_count() const {
            return m_opaque_commands.size() + m_transparent_commands.size();
        }

        static u64 calculate_sort_key(const RenderCommand& command);

      private:
        std::vector<RenderCommand> m_opaque_commands;
        std::vector<RenderCommand> m_transparent_commands;
    };
} // namespace star::rendering
