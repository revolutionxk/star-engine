#pragma once
#include <vector>

#include "star/core/types.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/resource_handle.hpp"
#include "star/math/math.hpp"
#include "star/rendering/frustum.hpp"

namespace star::graphics {
    struct Shader;
} // namespace star::graphics

namespace star::resources {
    class ResourceManager;
} // namespace star::resources

namespace star::rendering {
    struct DebugVertex {
        float x, y, z;
        u32 color;
    };

    static_assert(sizeof(DebugVertex) == 16);

    class DebugRenderer {
      public:
        explicit DebugRenderer(resources::ResourceManager& rm);
        ~DebugRenderer() = default;

        DebugRenderer(const DebugRenderer&) = delete;
        DebugRenderer& operator=(const DebugRenderer&) = delete;

        void draw_line(Vector3 from, Vector3 to, Color4 color = {1, 1, 1, 1});

        void draw_ray(Vector3 origin, Vector3 direction, f32 length = 1.0f, Color4 color = {1, 1, 1, 1});

        void draw_aabb(const AABB& aabb, Color4 color = {1, 1, 1, 1});

        void draw_box(Vector3 center, Vector3 half_extents, Quaternion rotation = Quaternion::identity(),
                      Color4 color = {1, 1, 1, 1});

        void draw_sphere(Vector3 center, f32 radius, Color4 color = {1, 1, 1, 1}, u32 segments = 16);

        void draw_frustum(const Matrix4& view_proj, Color4 color = {1, 1, 1, 1});

        void draw_axis(Vector3 pos, Quaternion rotation = Quaternion::identity(), f32 size = 1.0f);

        void draw_grid(Vector3 center, u32 cells = 10, f32 cell_size = 1.0f, Color4 color = {0.3f, 0.3f, 0.3f, 1.0f});

        void flush(graphics::DeviceContext& ctx, u32 view_id, const Matrix4& view, const Matrix4& proj);

        void new_frame();

        [[nodiscard]] bool has_lines() const noexcept {
            return !m_verts.empty();
        }

        [[nodiscard]] bool is_enabled() const noexcept {
            return m_enabled;
        }

        void set_enabled(const bool v) noexcept {
            m_enabled = v;
        }

      private:
        static u32 pack_rgba(Color4 c) noexcept;
        void push_line(Vector3 a, Vector3 b, u32 color);

        resources::ResourceManager& m_resource_manager;
        graphics::ResourceHandle<graphics::Shader> m_shader;

        std::vector<DebugVertex> m_verts;
        bool m_enabled{true};
    };

} // namespace star::rendering
