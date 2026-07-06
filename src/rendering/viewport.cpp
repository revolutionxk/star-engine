#include "star/rendering/viewport.hpp"

#include <bgfx/bgfx.h>

#include "star/core/common.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/graphics/texture.hpp"
#include "star/rendering/render_target.hpp"

namespace star::rendering {
    f32 halton(u32 index, const u32 base) {
        f32 f = 1.0f;
        f32 r = 0.0f;
        while (index > 0) {
            f /= static_cast<f32>(base);
            r += f * static_cast<f32>(index % base);
            index /= base;
        }
        return r;
    }

    Viewport::Viewport(graphics::Device& device) : m_device(&device) {
        STAR_LOG_INFO(LogCategory::Rendering, "Viewport created ({}x{})", m_width, m_height);
    }

    Viewport::~Viewport() {
        destroy_render_target();
        STAR_LOG_INFO(LogCategory::Rendering, "Viewport destroyed");
    }

    void Viewport::resize(const u32 width, const u32 height) {
        if (m_width == width && m_height == height) {
            return;
        }

        m_width = width;
        m_height = height;

        STAR_LOG_INFO(LogCategory::Rendering, "Viewport resized to {}x{}", width, height);

        if (m_framebuffer_enabled && m_render_target) {
            m_render_target->resize(width, height);
        }
        if (m_framebuffer_enabled && m_display_target) {
            m_display_target->resize(width, height);
        }
    }

    void Viewport::bind(graphics::DeviceContext& context, const u32 view_id) const {
        if (m_width == 0 || m_height == 0) {
            STAR_LOG_WARN(LogCategory::Rendering, "Cannot bind viewport with zero dimensions");
            return;
        }

        context.set_view_rect(view_id, 0, 0, static_cast<u16>(m_width), static_cast<u16>(m_height));
        context.set_view_clear(view_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);

        if (m_framebuffer_enabled && m_render_target && m_render_target->is_valid()) {
            context.set_view_framebuffer(view_id, m_render_target->framebuffer());
        } else {
            context.set_view_framebuffer(view_id, graphics::ResourceHandle<graphics::Framebuffer>{});
        }

        if (m_has_camera) {
            context.set_view_transform(view_id, m_view_matrix,
                                       m_taa_enabled ? m_render_projection : m_projection_matrix);
        }

        context.touch(view_id);
    }

    void Viewport::update_temporal() {
        m_prev_view_proj = m_cur_view_proj;
        m_cur_view_proj = m_projection_matrix * m_view_matrix;

        if (!m_taa_enabled || m_width == 0 || m_height == 0) {
            m_render_projection = m_projection_matrix;
            return;
        }

        ++m_taa_index;
        const f32 jx = (halton(m_taa_index, 2) - 0.5f) * 2.0f / static_cast<f32>(m_width);
        const f32 jy = (halton(m_taa_index, 3) - 0.5f) * 2.0f / static_cast<f32>(m_height);
        m_render_projection = m_projection_matrix;
        m_render_projection(2, 0) += jx;
        m_render_projection(2, 1) += jy;
    }

    void Viewport::bind_overlay(graphics::DeviceContext& context, const u32 view_id) const {
        if (m_width == 0 || m_height == 0)
            return;

        context.set_view_rect(view_id, 0, 0, static_cast<u16>(m_width), static_cast<u16>(m_height));
        context.set_view_clear(view_id, 0, 0, 1.0f, 0);

        if (m_framebuffer_enabled && m_render_target && m_render_target->is_valid()) {
            context.set_view_framebuffer(view_id, m_render_target->framebuffer());
        } else {
            context.set_view_framebuffer(view_id, graphics::ResourceHandle<graphics::Framebuffer>{});
        }

        if (m_has_camera) {
            context.set_view_transform(view_id, m_view_matrix,
                                       m_taa_enabled ? m_render_projection : m_projection_matrix);
        }
    }

    void Viewport::set_camera(const components::Camera& camera, const components::Transform& transform) {
        m_has_camera = true;
        update_camera_matrices(camera, transform);
    }

    void Viewport::set_framebuffer_enabled(const bool enabled) {
        if (m_framebuffer_enabled == enabled) {
            return;
        }

        m_framebuffer_enabled = enabled;

        if (enabled) {
            create_render_target();
            STAR_LOG_INFO(LogCategory::Rendering, "Viewport framebuffer mode enabled");
        } else {
            destroy_render_target();
            STAR_LOG_INFO(LogCategory::Rendering, "Viewport framebuffer mode disabled (rendering to backbuffer)");
        }
    }

    graphics::ResourceHandle<graphics::Texture> Viewport::get_color_texture() const {
        if (m_display_target && m_display_target->is_valid()) {
            return m_display_target->color_texture(0);
        }
        if (m_render_target && m_render_target->is_valid()) {
            return m_render_target->color_texture(0);
        }
        return {};
    }

    graphics::ResourceHandle<graphics::Texture> Viewport::hdr_color_texture() const {
        if (m_render_target && m_render_target->is_valid()) {
            return m_render_target->color_texture(0);
        }
        return {};
    }

    graphics::ResourceHandle<graphics::Texture> Viewport::hdr_normal_texture() const {
        if (m_render_target && m_render_target->is_valid()) {
            return m_render_target->color_texture(1);
        }
        return {};
    }

    graphics::ResourceHandle<graphics::Texture> Viewport::hdr_velocity_texture() const {
        if (m_render_target && m_render_target->is_valid()) {
            return m_render_target->color_texture(2);
        }
        return {};
    }

    graphics::ResourceHandle<graphics::Texture> Viewport::hdr_depth_texture() const {
        if (m_render_target && m_render_target->is_valid()) {
            return m_render_target->depth_texture();
        }
        return {};
    }

    graphics::ResourceHandle<graphics::Framebuffer> Viewport::display_framebuffer() const {
        if (m_display_target && m_display_target->is_valid()) {
            return m_display_target->framebuffer();
        }
        return {};
    }

    void Viewport::create_render_target() {
        if (m_width == 0 || m_height == 0) {
            STAR_LOG_WARN(LogCategory::Rendering, "Cannot create render target with zero dimensions");
            return;
        }

        m_render_target = std::make_unique<RenderTarget>();
        constexpr graphics::TextureFormat hdr_formats[3] = {
            graphics::TextureFormat::RGBA16F, graphics::TextureFormat::RGBA16F, graphics::TextureFormat::RGBA16F};
        const bool hdr_ok = m_render_target->create_mrt(m_device, m_width, m_height, hdr_formats, 3, true);

        m_display_target = std::make_unique<RenderTarget>();
        const bool ldr_ok =
            m_display_target->create(m_device, m_width, m_height, graphics::TextureFormat::RGBA8, false);

        if (!hdr_ok || !ldr_ok) {
            STAR_LOG_ERROR(LogCategory::Rendering, "Failed to create viewport render targets");
            m_render_target.reset();
            m_display_target.reset();
        } else {
            STAR_LOG_INFO(LogCategory::Rendering, "Viewport render targets created ({}x{})", m_width, m_height);
        }
    }

    void Viewport::destroy_render_target() {
        if (m_render_target) {
            m_render_target->destroy();
            m_render_target.reset();
        }
        if (m_display_target) {
            m_display_target->destroy();
            m_display_target.reset();
        }
    }

    bool Viewport::needs_uv_y_flip() {
        return bgfx::getCaps()->originBottomLeft;
    }

    void Viewport::update_camera_matrices(const components::Camera& camera, const components::Transform& transform) {
        m_camera_position = transform.position;

        const auto transform_matrix = transform.to_matrix();
        m_view_matrix = Matrix4::inverse(transform_matrix);

        const f32 aspect = aspect_ratio();
        m_projection_matrix = Matrix4::perspective(radians(camera.fov_y), aspect, camera.near_plane, camera.far_plane);

        STAR_LOG_TRACE(LogCategory::Rendering, "Viewport camera updated: pos({}, {}, {}), aspect={}",
                       m_camera_position.x, m_camera_position.y, m_camera_position.z, aspect);
    }

} // namespace star::rendering
