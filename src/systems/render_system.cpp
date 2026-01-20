#include "star/systems/render_system.hpp"

#include "star/core/common.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/components/material.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/render_queue.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/components/transform.hpp"
#include "star/scene/scene.hpp"

namespace star::systems {
    RenderSystem::RenderSystem(graphics::Device& device) : m_device(device) {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem initialized");

        m_view_matrix = Matrix4::identity();
        m_projection_matrix = Matrix4::identity();
        m_camera_position = Vector3{0.0f, 0.0f, 0.0f};
    }

    RenderSystem::~RenderSystem() {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem shutdown");
    }

    void RenderSystem::update(f32 delta_time) {}

    void RenderSystem::render(scene::Scene& scene, graphics::DeviceContext& context) {
        if (!scene.is_active()) {
            return;
        }

        if (m_viewport_width == 0 || m_viewport_height == 0) {
            STAR_LOG_WARN(LogCategory::Rendering, "Invalid viewport size: {}x{}", m_viewport_width, m_viewport_height);
            return;
        }

        m_render_queue.clear();

        setup_camera(scene);

        if (!m_has_camera) {
            STAR_LOG_WARN(LogCategory::Rendering, "No active camera found in scene '{}'", scene.name());
            return;
        }

        collect_renderables(scene);

        m_render_queue.sort();

        execute_render_queue(context);
    }

    void RenderSystem::setup_camera(scene::Scene& scene) {
        m_has_camera = false;

        const auto& world = scene.world();

        world.each([&](flecs::entity e, const scene::Camera& camera, const scene::Transform& transform) {
            if (!camera.is_primary) {
                return;
            }

            m_has_camera = true;
            m_camera_position = transform.position;

            const auto transform_matrix = transform.to_matrix();
            m_view_matrix = Matrix4::inverse(transform_matrix);

            const f32 aspect_ratio = static_cast<f32>(m_viewport_width) / static_cast<f32>(m_viewport_height);

            m_projection_matrix = Matrix4::perspective(camera.fov_y, aspect_ratio, camera.near_plane, camera.far_plane);

            STAR_LOG_TRACE(LogCategory::Rendering, "Camera setup: pos({}, {}, {})", transform.position.x,
                           transform.position.y, transform.position.z);
        });
    }

    void RenderSystem::collect_renderables(scene::Scene& scene) {
        const auto& world = scene.world();

        world.each([&](const flecs::entity e, const components::MeshRenderer& mesh_renderer,
                       const scene::Transform& transform) {
            if (!mesh_renderer.visible) {
                return;
            }

            if (!mesh_renderer.mesh.is_valid()) {
                return;
            }

            rendering::RenderCommand command;
            command.model_matrix = transform.to_matrix();
            command.mvp_matrix = m_projection_matrix * m_view_matrix * command.model_matrix;
            command.mesh = mesh_renderer.mesh;
            command.material = mesh_renderer.material;
            command.layer = mesh_renderer.layer;

            const Vector3 object_position = transform.position;
            const Vector3 delta = object_position - m_camera_position;
            command.distance_to_camera = delta.length();

            command.is_transparent = false;

            command.sort_key = rendering::RenderQueue::calculate_sort_key(command);

            m_render_queue.submit(command);

            STAR_LOG_TRACE(LogCategory::Rendering, "Submitted entity {} to render queue (dist: {:.2f})",
                           static_cast<u64>(e), command.distance_to_camera);
        });

        // STAR_LOG_DEBUG(LogCategory::Rendering, "Collected {} renderables for scene '{}'",
        //                m_render_queue.command_count(), scene.name());
    }

    void RenderSystem::execute_render_queue(graphics::DeviceContext& context) {
        context.set_view_rect(0, 0, 0, static_cast<u16>(m_viewport_width), static_cast<u16>(m_viewport_height));
        context.set_view_clear(0, 0x1 | 0x2, 0x443355FF, 1.0f, 0);
        context.set_view_transform(0, m_view_matrix, m_projection_matrix);

        if (!m_resource_manager) {
            STAR_LOG_WARN(LogCategory::Rendering, "ResourceManager not set, cannot render");
            return;
        }

        for (const auto& opaque_commands = m_render_queue.opaque_commands(); const auto& command : opaque_commands) {
            if (!command.mesh.is_valid()) {
                continue;
            }

            const auto* mesh = m_resource_manager->get_mesh(command.mesh);
            if (!mesh) {
                STAR_LOG_WARN(LogCategory::Rendering, "Failed to get mesh from handle");
                continue;
            }

            context.set_transform(command.model_matrix);
            if (mesh->vertex_buffer.is_valid() && mesh->index_buffer.is_valid()) {
                context.set_vertex_buffer(0, mesh->vertex_buffer);
                context.set_index_buffer(mesh->index_buffer);
            } else {
                STAR_LOG_WARN(LogCategory::Rendering, "Mesh has invalid buffers");
                continue;
            }

            graphics::ResourceHandle<graphics::Shader> shader_handle;
            if (command.material.is_valid()) {
                if (const auto* material = m_resource_manager->get_material(command.material);
                    material && material->shader.is_valid()) {
                    shader_handle = material->shader;
                }
            }

            if (!shader_handle.is_valid()) {
                if (auto default_shader_res = m_resource_manager->default_shader(); default_shader_res.is_valid()) {
                    if (const auto* default_shader = m_resource_manager->get_shader(default_shader_res)) {
                        shader_handle = default_shader->handle;
                    }
                }
            }

            if (shader_handle.is_valid()) {
                context.submit(0, shader_handle);
            } else {
                STAR_LOG_WARN(LogCategory::Rendering, "No valid shader available for rendering");
            }

            STAR_LOG_TRACE(LogCategory::Rendering, "Rendered opaque object (dist: {:.2f})", command.distance_to_camera);
        }

        for (const auto& transparent_commands = m_render_queue.transparent_commands();
             const auto& command : transparent_commands) {
            // TODO: Implementar renderização de transparentes com blending

            STAR_LOG_TRACE(LogCategory::Rendering, "Rendering transparent object (dist: {:.2f})",
                           command.distance_to_camera);
        }

        if (m_render_queue.command_count() > 0) {
            // STAR_LOG_DEBUG(LogCategory::Rendering, "Rendered {} opaque + {} transparent objects",
            //                opaque_commands.size(), transparent_commands.size());
        }
    }
} // namespace star::systems
