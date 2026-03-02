#pragma once
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/rendering/material_property.hpp"

namespace star::resources {
    struct Mesh;
    struct Material;
} // namespace star::resources

namespace star::rendering {
    struct DrawCall {
        Matrix4 model_matrix;
        graphics::ResourceHandle<resources::Mesh> mesh;
        graphics::ResourceHandle<resources::Material> material;
        std::vector<MaterialProperty> parameters;
        u64 sort_key{0};
        f32 distance_sq{0.0f};
        u8 layer{0};
        bool is_transparent{false};
    };

    class RenderQueue {
      public:
        RenderQueue();
        ~RenderQueue();

        void submit(const DrawCall& draw_call);
        void sort();
        void clear();

        const std::vector<DrawCall>& opaque_commands() const {
            return m_opaque_commands;
        }

        const std::vector<DrawCall>& transparent_commands() const {
            return m_transparent_commands;
        }

        size_t command_count() const {
            return m_opaque_commands.size() + m_transparent_commands.size();
        }

        static u64 calculate_sort_key(const DrawCall& draw_call);

      private:
        std::vector<DrawCall> m_opaque_commands;
        std::vector<DrawCall> m_transparent_commands;
    };
} // namespace star::rendering
