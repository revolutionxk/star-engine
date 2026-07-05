#pragma once

#include <memory>

#include "star/core/types.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/resource_handle.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
    struct Framebuffer;
    struct Texture;
} // namespace star::graphics

namespace star::rendering {
    class RenderTarget;

    class Viewport {
      public:
        explicit Viewport(graphics::Device& device);
        ~Viewport();

        Viewport(const Viewport&) = delete;
        Viewport& operator=(const Viewport&) = delete;

        void resize(u32 width, u32 height);

        void bind(graphics::DeviceContext& context, u32 view_id) const;
        void bind_overlay(graphics::DeviceContext& context, u32 view_id) const;

        void set_camera(const components::Camera& camera, const components::Transform& transform);
        void set_framebuffer_enabled(bool enabled);

        [[nodiscard]] graphics::ResourceHandle<graphics::Texture> get_color_texture() const;

        [[nodiscard]] RenderTarget* get_render_target() const {
            return m_render_target.get();
        }

        [[nodiscard]] u32 width() const {
            return m_width;
        }

        [[nodiscard]] u32 height() const {
            return m_height;
        }

        [[nodiscard]] f32 aspect_ratio() const {
            return m_width > 0 && m_height > 0 ? static_cast<f32>(m_width) / static_cast<f32>(m_height) : 16.0f / 9.0f;
        }

        [[nodiscard]] const Matrix4& view_matrix() const {
            return m_view_matrix;
        }

        [[nodiscard]] const Matrix4& projection_matrix() const {
            return m_projection_matrix;
        }

        [[nodiscard]] const Vector3& camera_position() const {
            return m_camera_position;
        }

        [[nodiscard]] bool has_camera() const {
            return m_has_camera;
        }

        [[nodiscard]] bool is_framebuffer_enabled() const {
            return m_framebuffer_enabled;
        }

        [[nodiscard]] static bool needs_uv_y_flip();

      private:
        void create_render_target();
        void destroy_render_target();
        void update_camera_matrices(const components::Camera& camera, const components::Transform& transform);

        graphics::Device* m_device;

        u32 m_width{1280};
        u32 m_height{720};

        bool m_framebuffer_enabled{false};
        std::unique_ptr<RenderTarget> m_render_target;

        Matrix4 m_view_matrix{Matrix4::identity()};
        Matrix4 m_projection_matrix{Matrix4::identity()};
        Vector3 m_camera_position{0.0f, 0.0f, 0.0f};
        bool m_has_camera{false};
    };

} // namespace star::rendering
