#include "star/rendering/systems/render_system.hpp"

#include "star/core/common.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/components/light.hpp"
#include "star/rendering/components/material_instance.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/rendering/frustum.hpp"
#include "star/rendering/material_property.hpp"
#include "star/rendering/render_queue.hpp"
#include "star/rendering/shader_uniforms.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"
#include "star/scene/components/camera.hpp"
#include "star/scene/scene.hpp"

namespace {
    void upload_property(graphics::DeviceContext& ctx, const rendering::MaterialProperty& prop) {
        std::visit(
            [&]<typename Value>(const Value& v) {
                using T = std::decay_t<Value>;

                if constexpr (std::is_same_v<T, float>) {
                    const Vector4 padded{v, 0.0f, 0.0f, 0.0f};
                    ctx.set_uniform(prop.name, &padded, 1, graphics::UniformType::Vec4);

                } else if constexpr (std::is_same_v<T, Vector2>) {
                    const Vector4 padded{v.x, v.y, 0.0f, 0.0f};
                    ctx.set_uniform(prop.name, &padded, 1, graphics::UniformType::Vec4);

                } else if constexpr (std::is_same_v<T, Vector3>) {
                    const Vector4 padded{v.x, v.y, v.z, 0.0f};
                    ctx.set_uniform(prop.name, &padded, 1, graphics::UniformType::Vec4);

                } else if constexpr (std::is_same_v<T, Vector4>) {
                    ctx.set_uniform(prop.name, &v, 1, graphics::UniformType::Vec4);

                } else if constexpr (std::is_same_v<T, Matrix4>) {
                    ctx.set_uniform(prop.name, &v, 1, graphics::UniformType::Mat4);

                } else if constexpr (std::is_same_v<T, graphics::ResourceHandle<graphics::Texture>>) {
                    if (v.is_valid())
                        ctx.set_texture(prop.texture_stage, v);
                }
            },
            prop.value);
    }

    void submit_material(graphics::DeviceContext& ctx, const resources::Material& mat,
                         const std::vector<rendering::MaterialProperty>& params) {
        ctx.set_pipeline_state(mat.pipeline_state);

        std::unordered_set<std::string_view> overridden;
        overridden.reserve(params.size());
        for (const auto& p : params)
            overridden.insert(p.name);

        if (!overridden.contains(rendering::uniforms::BASE_COLOR))
            ctx.set_uniform(std::string(rendering::uniforms::BASE_COLOR), &mat.albedo_color, 1,
                            graphics::UniformType::Vec4);

        const Vector4 pbr_params{mat.metallic, mat.roughness, 0.0f, 0.0f};
        if (!overridden.contains(rendering::uniforms::MATERIAL_PARAMS))
            ctx.set_uniform(std::string(rendering::uniforms::MATERIAL_PARAMS), &pbr_params, 1,
                            graphics::UniformType::Vec4);

        if (mat.albedo_texture.is_valid() && !overridden.contains(rendering::uniforms::ALBEDO_TEXTURE))
            ctx.set_texture(rendering::uniforms::STAGE_ALBEDO, mat.albedo_texture);

        for (const auto& prop : params)
            upload_property(ctx, prop);
    }

} // anonymous namespace

namespace star::systems {
    RenderSystem::RenderSystem(graphics::Device& device, resources::ResourceManager& resource_manager)
        : m_device(device), m_resource_manager(resource_manager) {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem initialized");
    }

    RenderSystem::~RenderSystem() {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem shutdown");
    }

    void RenderSystem::update(f32 delta_time) {}

    void RenderSystem::collect_lights(scene::Scene& scene) {
        m_light_env = {};

        scene.world().each([&](flecs::entity, const components::Light& light) {
            if (light.type != components::Light::Type::Directional || m_light_env.has_directional)
                return;

            m_light_env.directional_dir = light.direction;
            m_light_env.directional_color = light.color;
            m_light_env.directional_intensity = light.intensity;
            m_light_env.ambient_intensity = light.ambient_intensity;
            m_light_env.has_directional = true;
        });
    }

    void RenderSystem::submit_lighting(graphics::DeviceContext& context) const {
        const Vector4 light_dir{m_light_env.directional_dir.x, m_light_env.directional_dir.y,
                                m_light_env.directional_dir.z, 0.0f};
        context.set_uniform(std::string(rendering::uniforms::LIGHT_DIR), &light_dir, 1, graphics::UniformType::Vec4);

        const f32 i = m_light_env.directional_intensity;
        const Vector4 light_color{m_light_env.directional_color.x * i, m_light_env.directional_color.y * i,
                                  m_light_env.directional_color.z * i, 1.0f};
        context.set_uniform(std::string(rendering::uniforms::LIGHT_COLOR), &light_color, 1,
                            graphics::UniformType::Vec4);

        const Vector4 ambient_color{m_light_env.ambient_color.x, m_light_env.ambient_color.y,
                                    m_light_env.ambient_color.z, m_light_env.ambient_intensity};
        context.set_uniform(std::string(rendering::uniforms::AMBIENT_COLOR), &ambient_color, 1,
                            graphics::UniformType::Vec4);
    }

    void RenderSystem::render(scene::Scene& scene, graphics::DeviceContext& context, const u32 view_id,
                              rendering::Viewport* viewport) {
        if (!scene.is_active()) {
            return;
        }

        m_render_queue.clear();

        setup_camera(scene, viewport);
        collect_lights(scene);

        if (!viewport || !viewport->has_camera()) {
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
        const auto& camera_position = viewport->camera_position();
        const rendering::Frustum frustum =
            rendering::Frustum::extract(viewport->projection_matrix() * viewport->view_matrix());

        world.each([&](const flecs::entity e, const components::MeshRenderer& mesh_renderer,
                       const components::Transform& transform) {
            if (!mesh_renderer.visible || !mesh_renderer.mesh.is_valid()) {
                return;
            }

            rendering::DrawCall draw_call;
            draw_call.model_matrix = transform.to_matrix();
            draw_call.mesh = mesh_renderer.mesh;
            draw_call.material = mesh_renderer.material;
            draw_call.layer = mesh_renderer.layer;

            // Frustum cull against cached mesh AABB — skip invisible objects before building draw call
            if (const auto* mesh = m_resource_manager.get_mesh(mesh_renderer.mesh)) {
                if (!frustum.intersects_aabb_world(mesh->bounds, draw_call.model_matrix))
                    return;
            }

            if (const auto* material_instance = e.try_get<components::MaterialInstance>()) {
                draw_call.parameters = material_instance->parameters;
                if (material_instance->material.is_valid())
                    draw_call.material = material_instance->material;
            }

            draw_call.distance_sq = camera_position.distance_squared_to(transform.position);
            draw_call.is_transparent = false;
            draw_call.sort_key = rendering::RenderQueue::calculate_sort_key(draw_call);

            m_render_queue.submit(draw_call);

            STAR_LOG_TRACE(LogCategory::Rendering, "Submitted entity {} to render queue", static_cast<u64>(e));
        });
    }

    void RenderSystem::execute_render_queue(graphics::DeviceContext& context, const u32 view_id,
                                            const rendering::Viewport* viewport) {
        if (viewport) {
            viewport->bind(context, view_id);
        }

        submit_lighting(context);

        for (const auto& opaque_commands = m_render_queue.opaque_commands(); const auto& command : opaque_commands) {
            if (!command.mesh.is_valid()) {
                continue;
            }

            const auto* mesh = m_resource_manager.get_mesh(command.mesh);
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
                if (const auto* material = m_resource_manager.get_material(command.material)) {
                    submit_material(context, *material, command.parameters);

                    // material->shader holds the resource manager slot ID — resolve to get the actual GPU handle
                    if (material->shader.is_valid()) {
                        if (const auto* shader_res = m_resource_manager.get_shader(material->shader)) {
                            shader_handle = shader_res->handle;
                        }
                    }
                }
            }

            if (!shader_handle.is_valid()) {
                if (auto default_shader_res = m_resource_manager.default_shader(); default_shader_res.is_valid()) {
                    if (const auto* default_shader = m_resource_manager.get_shader(default_shader_res)) {
                        shader_handle = default_shader->handle;
                    }
                }
            }

            if (shader_handle.is_valid()) {
                context.submit(view_id, shader_handle);
            } else {
                STAR_LOG_WARN(LogCategory::Rendering, "No valid shader available for rendering");
            }

            STAR_LOG_TRACE(LogCategory::Rendering, "Rendered opaque object (dist_sq: {:.2f})", command.distance_sq);
        }

        // TODO: transparent pass — sort back-to-front, enable blending per pipeline state
    }
} // namespace star::systems
