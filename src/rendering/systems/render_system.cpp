#include "star/rendering/systems/render_system.hpp"

#include "star/core/common.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/components/material.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/render_queue.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/scene.hpp"

namespace star::systems {
    RenderSystem::RenderSystem(graphics::Device& device) : m_device(device) {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem initialized");
    }

    RenderSystem::~RenderSystem() {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem shutdown");
    }

    void RenderSystem::update(f32 delta_time) {}

    void RenderSystem::render(scene::Scene& scene, graphics::DeviceContext& context, const u32 view_id,
                              rendering::Viewport* viewport) {
        if (!scene.is_active()) {
            return;
        }

        m_render_queue.clear();

        setup_camera(scene, viewport);

        if (const bool has_camera = viewport ? viewport->has_camera() : false; !has_camera) {
            STAR_LOG_WARN(LogCategory::Rendering, "No active camera found in scene '{}'", scene.name());
            return;
        }

        collect_renderables(scene, viewport);

        m_render_queue.sort();

        execute_render_queue(context, view_id, viewport);
    }

    void RenderSystem::setup_camera(scene::Scene& scene, rendering::Viewport* viewport) {
        if (!viewport) {
            return;
        }

        const auto& world = scene.world();

        world.each([&](flecs::entity, const components::Camera& camera, const components::Transform& transform,
                       const components::PrimaryCamera&) {
            viewport->set_camera(camera, transform);

            STAR_LOG_TRACE(LogCategory::Rendering, "Camera setup: pos({}, {}, {})", transform.position.x,
                           transform.position.y, transform.position.z);
        });
    }

    void RenderSystem::collect_renderables(scene::Scene& scene, const rendering::Viewport* viewport) {
        if (!viewport) {
            return;
        }

        const auto& world = scene.world();
        const auto& view_matrix = viewport->view_matrix();
        const auto& projection_matrix = viewport->projection_matrix();
        const auto& camera_position = viewport->camera_position();

        world.each([&](const flecs::entity e, const components::MeshRenderer& mesh_renderer,
                       const components::Transform& transform) {
            if (!mesh_renderer.visible) {
                return;
            }

            if (!mesh_renderer.mesh.is_valid()) {
                return;
            }

            rendering::DrawCall draw_call;
            draw_call.model_matrix = transform.to_matrix();
            draw_call.mvp_matrix = projection_matrix * view_matrix * draw_call.model_matrix;
            draw_call.mesh = mesh_renderer.mesh;
            draw_call.material = mesh_renderer.material;
            draw_call.layer = mesh_renderer.layer;

            const Vector3 object_position = transform.position;
            const Vector3 delta = object_position - camera_position;
            draw_call.distance_to_camera = delta.length();

            draw_call.is_transparent = false;

            draw_call.sort_key = rendering::RenderQueue::calculate_sort_key(draw_call);

            m_render_queue.submit(draw_call);

            STAR_LOG_TRACE(LogCategory::Rendering, "Submitted entity {} to render queue (dist: {:.2f})",
                           static_cast<u64>(e), draw_call.distance_to_camera);
        });

        // STAR_LOG_DEBUG(LogCategory::Rendering, "Collected {} renderables for scene '{}'",
        //                m_render_queue.command_count(), scene.name());
    }

    void RenderSystem::execute_render_queue(graphics::DeviceContext& context, const u32 view_id,
                                            const rendering::Viewport* viewport) const {
        if (viewport) {
            viewport->bind(context, view_id);
        }

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
                context.submit(view_id, shader_handle);
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
