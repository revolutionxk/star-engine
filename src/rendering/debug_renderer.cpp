#include "star/rendering/debug_renderer.hpp"

#include <cmath>
#include <numbers>

#include "star/core/common.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/pipeline_state.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/builtin_shaders.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {

    DebugRenderer::DebugRenderer(resources::ResourceManager& rm) : m_resource_manager(rm) {
        m_verts.reserve(4096);

        if (const auto slot = rm.register_builtin_shader("__debug_shader", resources::BuiltinShader::Debug); slot.is_valid()) {
            if (const auto* shader = rm.get_shader(slot)) {
                m_shader = shader->handle;
                STAR_LOG_INFO(LogCategory::Rendering, "DebugRenderer shader loaded");
            }
        } else {
            STAR_LOG_ERROR(LogCategory::Rendering, "DebugRenderer failed to load debug shader");
        }
    }

    u32 DebugRenderer::pack_rgba(const Color4 c) noexcept {
        const auto r = static_cast<u8>(std::clamp(c.x, 0.0f, 1.0f) * 255.0f);
        const auto g = static_cast<u8>(std::clamp(c.y, 0.0f, 1.0f) * 255.0f);
        const auto b = static_cast<u8>(std::clamp(c.z, 0.0f, 1.0f) * 255.0f);
        const auto a = static_cast<u8>(std::clamp(c.w, 0.0f, 1.0f) * 255.0f);
        return static_cast<u32>(r) | (static_cast<u32>(g) << 8) | (static_cast<u32>(b) << 16) |
               (static_cast<u32>(a) << 24);
    }

    void DebugRenderer::push_line(const Vector3 a, const Vector3 b, const u32 color) {
        m_verts.push_back({a.x, a.y, a.z, color});
        m_verts.push_back({b.x, b.y, b.z, color});
    }

    void DebugRenderer::draw_line(const Vector3 from, const Vector3 to, const Color4 color) {
        push_line(from, to, pack_rgba(color));
    }

    void DebugRenderer::draw_ray(const Vector3 origin, const Vector3 direction, const f32 length, const Color4 color) {
        draw_line(origin, origin + direction.normalized() * length, color);
    }

    void DebugRenderer::draw_aabb(const AABB& aabb, const Color4 color) {
        const u32 c = pack_rgba(color);
        const auto& mn = aabb.min;
        const auto& mx = aabb.max;

        // Bottom face
        push_line({mn.x, mn.y, mn.z}, {mx.x, mn.y, mn.z}, c);
        push_line({mx.x, mn.y, mn.z}, {mx.x, mn.y, mx.z}, c);
        push_line({mx.x, mn.y, mx.z}, {mn.x, mn.y, mx.z}, c);
        push_line({mn.x, mn.y, mx.z}, {mn.x, mn.y, mn.z}, c);
        // Top face
        push_line({mn.x, mx.y, mn.z}, {mx.x, mx.y, mn.z}, c);
        push_line({mx.x, mx.y, mn.z}, {mx.x, mx.y, mx.z}, c);
        push_line({mx.x, mx.y, mx.z}, {mn.x, mx.y, mx.z}, c);
        push_line({mn.x, mx.y, mx.z}, {mn.x, mx.y, mn.z}, c);
        // Vertical edges
        push_line({mn.x, mn.y, mn.z}, {mn.x, mx.y, mn.z}, c);
        push_line({mx.x, mn.y, mn.z}, {mx.x, mx.y, mn.z}, c);
        push_line({mx.x, mn.y, mx.z}, {mx.x, mx.y, mx.z}, c);
        push_line({mn.x, mn.y, mx.z}, {mn.x, mx.y, mx.z}, c);
    }

    void DebugRenderer::draw_box(const Vector3 center, const Vector3 half_extents, const Quaternion rotation,
                                 const Color4 color) {
        const u32 c = pack_rgba(color);

        // 8 corners of the unit box, scaled and rotated
        const Vector3 he = half_extents;
        const Vector3 corners[8] = {
            center + rotation * Vector3{-he.x, -he.y, -he.z}, center + rotation * Vector3{+he.x, -he.y, -he.z},
            center + rotation * Vector3{+he.x, -he.y, +he.z}, center + rotation * Vector3{-he.x, -he.y, +he.z},
            center + rotation * Vector3{-he.x, +he.y, -he.z}, center + rotation * Vector3{+he.x, +he.y, -he.z},
            center + rotation * Vector3{+he.x, +he.y, +he.z}, center + rotation * Vector3{-he.x, +he.y, +he.z},
        };

        // Bottom face
        push_line(corners[0], corners[1], c);
        push_line(corners[1], corners[2], c);
        push_line(corners[2], corners[3], c);
        push_line(corners[3], corners[0], c);
        // Top face
        push_line(corners[4], corners[5], c);
        push_line(corners[5], corners[6], c);
        push_line(corners[6], corners[7], c);
        push_line(corners[7], corners[4], c);
        // Verticals
        push_line(corners[0], corners[4], c);
        push_line(corners[1], corners[5], c);
        push_line(corners[2], corners[6], c);
        push_line(corners[3], corners[7], c);
    }

    void DebugRenderer::draw_sphere(const Vector3 center, const f32 radius, const Color4 color, const u32 segments) {
        const u32 c = pack_rgba(color);
        const f32 step = 2.0f * std::numbers::pi_v<f32> / static_cast<f32>(segments);

        for (u32 i = 0; i < segments; ++i) {
            const f32 a0 = static_cast<f32>(i) * step;
            const f32 a1 = a0 + step;
            const f32 c0 = std::cos(a0) * radius, s0 = std::sin(a0) * radius;
            const f32 c1 = std::cos(a1) * radius, s1 = std::sin(a1) * radius;

            push_line({center.x + c0, center.y + s0, center.z}, {center.x + c1, center.y + s1, center.z}, c);
            push_line({center.x + c0, center.y, center.z + s0}, {center.x + c1, center.y, center.z + s1}, c);
            push_line({center.x, center.y + c0, center.z + s0}, {center.x, center.y + c1, center.z + s1}, c);
        }
    }

    void DebugRenderer::draw_frustum(const Matrix4& view_proj, const Color4 color) {
        const u32 c = pack_rgba(color);
        const Matrix4 inv = view_proj.inversed();

        // 8 NDC corners (OpenGL: z in [-1, 1])
        // 0-3: near, 4-7: far
        const Vector3 ndc[8] = {
            {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1}, {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1},
        };

        Vector3 corners[8];
        for (int i = 0; i < 8; ++i)
            corners[i] = inv.transform_point(ndc[i]);

        // Near face
        for (int i = 0; i < 4; ++i)
            push_line(corners[i], corners[(i + 1) % 4], c);
        // Far face
        for (int i = 4; i < 8; ++i)
            push_line(corners[i], corners[4 + (i - 4 + 1) % 4], c);
        // Connecting edges
        for (int i = 0; i < 4; ++i)
            push_line(corners[i], corners[i + 4], c);
    }

    void DebugRenderer::draw_axis(const Vector3 pos, const Quaternion rotation, const f32 size) {
        push_line(pos, pos + rotation * Vector3{size, 0, 0}, pack_rgba({1, 0, 0, 1})); // X — red
        push_line(pos, pos + rotation * Vector3{0, size, 0}, pack_rgba({0, 1, 0, 1})); // Y — green
        push_line(pos, pos + rotation * Vector3{0, 0, size}, pack_rgba({0, 0, 1, 1})); // Z — blue
    }

    void DebugRenderer::draw_grid(const Vector3 center, const u32 cells, const f32 cell_size, const Color4 color) {
        const u32 c = pack_rgba(color);
        const f32 half = static_cast<f32>(cells) * cell_size * 0.5f;

        for (u32 i = 0; i <= cells; ++i) {
            const f32 t = -half + static_cast<f32>(i) * cell_size;
            push_line({center.x - half, center.y, center.z + t}, {center.x + half, center.y, center.z + t}, c);
            push_line({center.x + t, center.y, center.z - half}, {center.x + t, center.y, center.z + half}, c);
        }
    }

    void DebugRenderer::flush(graphics::DeviceContext& ctx, const u32 view_id, const Matrix4& /*view*/,
                              const Matrix4& /*proj*/) {
        if (m_verts.empty() || !m_enabled || !m_shader.is_valid())
            return;

        graphics::PipelineState ps;
        ps.blend_mode = graphics::BlendMode::Opaque;
        ps.cull = graphics::CullMode::None;
        ps.depth_test = graphics::DepthTest::LessEqual;
        ps.depth_write = false;
        ps.primitive = graphics::PrimitiveType::Lines;
        ctx.set_pipeline_state(ps);

        ctx.set_transform(Matrix4::identity());
        ctx.set_transient_vertex_buffer(0, m_verts.data(), static_cast<u32>(m_verts.size()),
                                        graphics::VertexLayoutType::Debug);
        ctx.submit(view_id, m_shader);
    }

    void DebugRenderer::new_frame() {
        m_verts.clear();
    }

} // namespace star::rendering
