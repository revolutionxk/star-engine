#include "star/rendering/passes/shadow_pass.hpp"

#include <cmath>

#include <bgfx/bgfx.h>

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

    ShadowPass::~ShadowPass() = default;

    bool ShadowPass::find_sun_direction(const RenderScene& scene, Vector3& out_travel_dir) {
        for (const auto& light : scene.lights) {
            if (light.type == LightSnapshot::Type::Directional && light.cast_shadows) {
                out_travel_dir = light.direction;
                return true;
            }
        }
        return false;
    }

    void ShadowPass::compute_light_matrices(const Vector3& sun_travel_dir, const Vector3& center, const f32 coverage,
                                            const bool homogeneous_depth, Matrix4& out_view, Matrix4& out_proj) {
        Vector3 light_dir = sun_travel_dir;
        if (light_dir.length_squared() < 1e-6f)
            light_dir = Vector3{0.0f, -1.0f, 0.0f};
        light_dir = light_dir.normalized();

        const f32 distance = coverage * 2.5f;
        constexpr f32 near_plane = 1.0f;
        const f32 far_plane = coverage * 5.0f;

        const Vector3 up = std::abs(light_dir.y) > 0.99f ? Vector3{0.0f, 0.0f, 1.0f} : Vector3{0.0f, 1.0f, 0.0f};
        const Vector3 eye = center - light_dir * distance;
        out_view = Matrix4::look_at(eye, center, up);

        if (homogeneous_depth) {
            out_proj = Matrix4::orthographic(-coverage, coverage, -coverage, coverage, near_plane, far_plane);
        } else {
            out_proj = Matrix4::identity();
            out_proj(0, 0) = 1.0f / coverage;
            out_proj(1, 1) = 1.0f / coverage;
            out_proj(2, 2) = -1.0f / (far_plane - near_plane);
            out_proj(3, 2) = -near_plane / (far_plane - near_plane);
        }

        const Matrix4 light_vp = out_proj * out_view;
        const Vector4 origin = light_vp * Vector4{center.x, center.y, center.z, 1.0f};
        constexpr f32 half_size = static_cast<f32>(SHADOW_MAP_SIZE) * 0.5f;
        const f32 tx = origin.x / origin.w * half_size;
        const f32 ty = origin.y / origin.w * half_size;
        out_proj(3, 0) += (std::round(tx) - tx) / half_size;
        out_proj(3, 1) += (std::round(ty) - ty) / half_size;
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
            m_target.create(&m_device, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, graphics::TextureFormat::RGBA16F, true);
        if (!m_target.is_valid()) {
            m_render_system.set_shadow(state);
            return;
        }

        Matrix4 light_view;
        Matrix4 light_proj;
        compute_light_matrices(sun_travel_dir, viewport->camera_position(), m_settings.coverage,
                               m_device.caps().homogeneous_depth, light_view, light_proj);

        auto& gpu = ctx.gpu;
        const u32 view = ctx.view_id;
        gpu.set_view_framebuffer(view, m_target.framebuffer());
        gpu.set_view_rect(view, 0, 0, static_cast<u16>(SHADOW_MAP_SIZE), static_cast<u16>(SHADOW_MAP_SIZE));
        gpu.set_view_clear(view, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0xFFFFFFFF, 1.0f, 0);
        gpu.set_view_transform(view, light_view, light_proj);

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

        state.map = m_target.color_texture(0);
        state.light_view_proj = light_proj * light_view;
        state.enabled = true;
        state.bias = m_settings.depth_bias;
        state.texel_size = 1.0f / static_cast<f32>(SHADOW_MAP_SIZE);
        state.origin_bottom_left = m_device.caps().origin_bottom_left;
        m_render_system.set_shadow(state);
    }
} // namespace star::rendering
