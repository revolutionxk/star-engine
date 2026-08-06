#include "star/rendering/passes/shadow_pass.hpp"

#include <algorithm>
#include <cmath>

#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/systems/render_system.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/mesh/mesh.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    ShadowPass::ShadowPass(graphics::Device& device, resources::ResourceManager& resources,
                           systems::RenderSystem& render_system)
        : m_device(device), m_resources(resources), m_render_system(render_system) {}

    ShadowPass::~ShadowPass() {
        m_target.destroy();
    }

    bool ShadowPass::find_sun_direction(const RenderScene& scene, Vector3& out_travel_dir) {
        for (const auto& light : scene.lights) {
            if (light.type == LightSnapshot::Type::Directional && light.cast_shadows) {
                out_travel_dir = light.direction;
                return true;
            }
        }
        return false;
    }

    u32 ShadowPass::active_cascade_count() const {
        return std::clamp(m_settings.cascade_count, 1u, MAX_SHADOW_CASCADES);
    }

    void ShadowPass::tile_offset(const u32 index, u32& out_x, u32& out_y) {
        out_x = (index % 2) * CASCADE_TILE_SIZE;
        out_y = (index / 2) * CASCADE_TILE_SIZE;
    }

    void ShadowPass::compute_splits(const f32 near_plane, const f32 far_plane,
                                    std::array<f32, MAX_SHADOW_CASCADES + 1>& out) const {
        const u32 count = active_cascade_count();
        const f32 range_far = std::min(far_plane, std::max(near_plane + 1.0f, m_settings.max_distance));
        const f32 lambda = std::clamp(m_settings.split_lambda, 0.0f, 1.0f);

        out[0] = near_plane;
        for (u32 i = 1; i <= count; ++i) {
            const f32 ratio = static_cast<f32>(i) / static_cast<f32>(count);
            const f32 logarithmic = near_plane * std::pow(range_far / near_plane, ratio);
            const f32 uniform = near_plane + (range_far - near_plane) * ratio;
            out[i] = lambda * logarithmic + (1.0f - lambda) * uniform;
        }
    }

    ShadowPass::Cascade ShadowPass::fit_cascade(const Viewport& viewport, const Vector3& light_dir,
                                                const f32 split_near, const f32 split_far) const {
        const Matrix4 inv_view_proj = Matrix4::inverse(viewport.projection_matrix() * viewport.view_matrix());
        const bool homogeneous = m_device.caps().homogeneous_depth;

        const f32 near_z = homogeneous ? -1.0f : 0.0f;
        constexpr f32 far_z = 1.0f;

        std::array<Vector3, 8> corners{};
        u32 index = 0;
        for (i32 x = 0; x < 2; ++x) {
            for (i32 y = 0; y < 2; ++y) {
                for (i32 z = 0; z < 2; ++z) {
                    const Vector4 ndc{static_cast<f32>(x) * 2.0f - 1.0f, static_cast<f32>(y) * 2.0f - 1.0f,
                                      z == 0 ? near_z : far_z, 1.0f};
                    const Vector4 world = inv_view_proj * ndc;
                    corners[index++] = Vector3{world.x, world.y, world.z} / world.w;
                }
            }
        }

        const f32 total_near = viewport.near_plane();
        const f32 total_far = viewport.far_plane();
        const f32 range = std::max(total_far - total_near, 1e-4f);
        const f32 t_near = std::clamp((split_near - total_near) / range, 0.0f, 1.0f);
        const f32 t_far = std::clamp((split_far - total_near) / range, 0.0f, 1.0f);

        std::array<Vector3, 8> slice{};
        for (u32 i = 0; i < 4; ++i) {
            const Vector3 ray = corners[i * 2 + 1] - corners[i * 2];
            slice[i] = corners[i * 2] + ray * t_near;
            slice[i + 4] = corners[i * 2] + ray * t_far;
        }

        Vector3 center{0.0f, 0.0f, 0.0f};
        for (const Vector3& corner : slice) {
            center += corner;
        }
        center = center / 8.0f;

        f32 radius = 0.0f;
        for (const Vector3& corner : slice) {
            radius = std::max(radius, (corner - center).length());
        }
        radius = std::ceil(radius * 16.0f) / 16.0f;

        const f32 texel_world = radius * 2.0f / static_cast<f32>(CASCADE_TILE_SIZE);

        const Vector3 up = std::abs(light_dir.y) > 0.99f ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{0.0f, 1.0f, 0.0f};
        const Vector3 eye = center - light_dir * (radius * 2.0f);

        const Matrix4 light_view = Matrix4::look_at(eye, center, up);
        Matrix4 light_proj =
            Matrix4::orthographic(-radius, radius, -radius, radius, 0.0f, radius * 4.0f, homogeneous);

        const Matrix4 unsnapped = light_proj * light_view;
        const Vector4 origin = unsnapped * Vector4{0.0f, 0.0f, 0.0f, 1.0f};
        constexpr f32 half_size = static_cast<f32>(CASCADE_TILE_SIZE) * 0.5f;
        const f32 tx = origin.x * half_size;
        const f32 ty = origin.y * half_size;
        light_proj(3, 0) += (std::round(tx) - tx) / half_size;
        light_proj(3, 1) += (std::round(ty) - ty) / half_size;

        Cascade cascade;
        cascade.view_proj = light_proj * light_view;
        cascade.split_far = split_far;
        cascade.texel_world_size = texel_world;
        cascade.depth_range = std::max(radius * 4.0f, 1e-3f);
        return cascade;
    }

    void ShadowPass::render(const RenderContext& ctx) {
        if (ctx.frame.frame_index == m_last_render_frame)
            return;
        m_last_render_frame = ctx.frame.frame_index;

        systems::RenderSystem::ShadowState state;
        state.enabled = false;

        const RenderScene* scene = ctx.frame.scene;
        const Viewport* viewport = ctx.frame.viewport;
        if (!m_settings.enabled || !scene || !scene->active || !viewport || !viewport->has_camera()) {
            m_render_system.set_shadow(state);
            return;
        }

        Vector3 sun_travel_dir;
        if (!find_sun_direction(*scene, sun_travel_dir)) {
            m_render_system.set_shadow(state);
            return;
        }

        if (sun_travel_dir.length_squared() < 1e-6f) {
            sun_travel_dir = Vector3{0.0f, -1.0f, 0.0f};
        }
        const Vector3 light_dir = sun_travel_dir.normalized();

        if (!m_shadow_shader.is_valid()) {
            const auto res = m_resources.register_builtin_shader("__shadow_shader", resources::BuiltinShader::Shadow);
            if (const auto* shader = m_resources.get_shader(res))
                m_shadow_shader = shader->handle;
        }
        if (!m_shadow_shader.is_valid()) {
            m_render_system.set_shadow(state);
            return;
        }

        if (!m_target.is_valid())
            m_target.create(&m_device, ATLAS_SIZE, ATLAS_SIZE, graphics::TextureFormat::RGBA16F, true);
        if (!m_target.is_valid()) {
            m_render_system.set_shadow(state);
            return;
        }

        const u32 count = active_cascade_count();

        std::array<f32, MAX_SHADOW_CASCADES + 1> splits{};
        compute_splits(viewport->near_plane(), viewport->far_plane(), splits);

        auto& gpu = ctx.gpu;

        const u32 clear_view = ctx.view_id;
        gpu.set_view_framebuffer(clear_view, m_target.framebuffer());
        gpu.set_view_rect(clear_view, 0, 0, static_cast<u16>(ATLAS_SIZE), static_cast<u16>(ATLAS_SIZE));
        gpu.set_view_clear(clear_view, graphics::ClearFlags::ColorDepth, 0xFFFFFFFF, 1.0f, 0);
        gpu.touch(clear_view);

        for (u32 c = 0; c < count; ++c) {
            const Cascade cascade = fit_cascade(*viewport, light_dir, splits[c], splits[c + 1]);

            u32 tile_x = 0;
            u32 tile_y = 0;
            tile_offset(c, tile_x, tile_y);

            const u32 view = ctx.frame.views.acquire("ShadowCascade");
            gpu.set_view_framebuffer(view, m_target.framebuffer());
            gpu.set_view_rect(view, static_cast<u16>(tile_x), static_cast<u16>(tile_y),
                              static_cast<u16>(CASCADE_TILE_SIZE), static_cast<u16>(CASCADE_TILE_SIZE));
            gpu.set_view_clear(view, graphics::ClearFlags::None, 0, 1.0f, 0);
            gpu.set_view_transform(view, Matrix4::identity(), cascade.view_proj);

            for (const auto& renderable : scene->renderables) {
                if (!renderable.mesh.is_valid())
                    continue;
                const auto* mesh = m_resources.get_mesh(renderable.mesh);
                if (!mesh || !mesh->vertex_buffer.is_valid() || !mesh->index_buffer.is_valid())
                    continue;

                gpu.set_transform(renderable.model_matrix);
                gpu.set_vertex_buffer(0, mesh->vertex_buffer);
                gpu.set_index_buffer(mesh->index_buffer);
                gpu.submit(view, m_shadow_shader);
            }

            state.cascades[c].view_proj = cascade.view_proj;
            state.cascades[c].split_far = cascade.split_far;
            state.cascades[c].texel_world_size = cascade.texel_world_size;
            state.cascades[c].depth_range = cascade.depth_range;
            state.cascades[c].atlas_offset = {static_cast<f32>(tile_x) / static_cast<f32>(ATLAS_SIZE),
                                              static_cast<f32>(tile_y) / static_cast<f32>(ATLAS_SIZE)};
        }

        state.map = m_target.color_texture(0);
        state.enabled = true;
        state.cascade_count = count;
        state.bias = m_settings.depth_bias;
        state.normal_bias = m_settings.normal_bias;
        state.atlas_scale = static_cast<f32>(CASCADE_TILE_SIZE) / static_cast<f32>(ATLAS_SIZE);
        state.texel_size = 1.0f / static_cast<f32>(ATLAS_SIZE);
        state.origin_bottom_left = m_device.caps().origin_bottom_left;
        state.visualize = m_settings.visualize_cascades;
        m_render_system.set_shadow(state);
    }
} // namespace star::rendering
