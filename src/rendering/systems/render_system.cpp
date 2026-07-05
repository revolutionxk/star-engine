#include "star/rendering/systems/render_system.hpp"

#include "star/core/common.hpp"
#include "star/ecs/components/camera.hpp"
#include "star/ecs/components/transform.hpp"
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/frustum.hpp"
#include "star/rendering/material_property.hpp"
#include "star/rendering/render_queue.hpp"
#include "star/rendering/render_scene.hpp"
#include "star/rendering/shader_uniforms.hpp"
#include "star/rendering/viewport.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace {
    constexpr f32 MAX_SPOT_INNER_HALF_ANGLE_DEG = 89.0f;
    constexpr f32 MAX_SPOT_OUTER_HALF_ANGLE_DEG = 89.9f;
    constexpr f32 MIN_SPOT_ANGLE_GAP_DEG = 0.1f;

    [[nodiscard]] f32 spot_half_angle_cos(const f32 angle_deg) {
        return std::cos(radians(angle_deg));
    }

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
                         const std::vector<rendering::MaterialProperty>& params, resources::ResourceManager& rm) {
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

        if (!overridden.contains(rendering::uniforms::EMISSIVE_COLOR))
            ctx.set_uniform(std::string(rendering::uniforms::EMISSIVE_COLOR), &mat.emissive_color, 1,
                            graphics::UniformType::Vec4);

        if (mat.albedo_texture.is_valid() && !overridden.contains(rendering::uniforms::ALBEDO_TEXTURE)) {
            if (const auto* tex = rm.get_texture(mat.albedo_texture))
                ctx.set_texture(rendering::uniforms::STAGE_ALBEDO, tex->handle);
        }

        for (const auto& prop : params)
            upload_property(ctx, prop);
    }
} // namespace

namespace star::systems {
    RenderSystem::RenderSystem(graphics::Device& device, resources::ResourceManager& resource_manager)
        : m_device(device), m_resource_manager(resource_manager) {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem initialized");
    }

    RenderSystem::~RenderSystem() {
        STAR_LOG_INFO(LogCategory::Rendering, "RenderSystem shutdown");
    }

    void RenderSystem::collect_lights(const rendering::RenderScene& scene) {
        m_light_env = {};
        auto& [pos_type, dir_range, color_int, cone, count] = m_light_env.lights;

        bool scene_has_directional = false;
        for (const auto& light : scene.lights) {
            if (light.type == rendering::LightSnapshot::Type::Directional) {
                scene_has_directional = true;
                break;
            }
        }

        const f32 sky_ambient_intensity = scene_has_directional ? 0.02f : 0.08f;
        const Vector3 ground_bounce =
            scene_has_directional ? Vector3{0.015f, 0.012f, 0.010f} : Vector3{0.035f, 0.030f, 0.025f};

        const auto push_light = [&](const Vector3& pos, const Vector3& dir, const Color4& color, const f32 intensity,
                                    const f32 type, const f32 range, const f32 inner_cos, const f32 outer_cos) {
            if (count >= rendering::uniforms::MAX_LIGHTS)
                return;
            const u32 i = count++;
            pos_type[i] = {pos.x, pos.y, pos.z, type};
            dir_range[i] = {dir.x, dir.y, dir.z, range};
            color_int[i] = {color.x, color.y, color.z, std::max(intensity, 0.0f)};
            cone[i] = {inner_cos, outer_cos, 0.0f, 0.0f};
        };

        bool directional_consumed = false;
        if (m_atmospheric.valid) {
            m_light_env.ambient_color = m_atmospheric.sky_color_rgb;
            m_light_env.ambient_intensity = sky_ambient_intensity;
            m_light_env.ground_color = ground_bounce;
            m_light_env.exposure = 0.85f;
            m_light_env.has_sky = true;
            m_light_env.has_sun = true;
            m_light_env.sun_direction = m_atmospheric.sun_direction;
            m_light_env.sun_luminance_rgb = m_atmospheric.sun_color_rgb * m_atmospheric.sun_intensity;
            m_light_env.sky_luminance_rgb = m_atmospheric.sky_color_rgb;

            if (!scene_has_directional) {
                push_light({0, 0, 0}, m_atmospheric.directional_dir,
                           Color4{m_atmospheric.sun_color_rgb.x, m_atmospheric.sun_color_rgb.y,
                                  m_atmospheric.sun_color_rgb.z, 1.0f},
                           m_atmospheric.sun_intensity, 0.0f, 0.0f, 0.0f, 0.0f);
                directional_consumed = true;
            }
        } else {
            m_light_env.ambient_color = {0.45f, 0.55f, 0.7f};
            m_light_env.ambient_intensity = 0.08f;
            m_light_env.ground_color = {0.035f, 0.03f, 0.025f};
            m_light_env.exposure = 0.85f;
            m_light_env.sky_luminance_rgb = m_light_env.ambient_color;
            m_light_env.sun_luminance_rgb = {0.0f, 0.0f, 0.0f};
        }

        for (const auto& light : scene.lights) {
            const Color4 light_color = light.color;
            f32 type_id = 0.0f;
            f32 inner_cone_cos = 0.0f;
            f32 outer_cone_cos = 0.0f;
            switch (light.type) {
                case rendering::LightSnapshot::Type::Directional:
                    if (directional_consumed)
                        continue;
                    type_id = 0.0f;
                    directional_consumed = true;
                    m_light_env.has_sun = true;
                    m_light_env.sun_direction = -light.direction;
                    m_light_env.sun_luminance_rgb = {light_color.x * light.intensity, light_color.y * light.intensity,
                                                     light_color.z * light.intensity};

                    if (!m_light_env.has_sky) {
                        m_light_env.ambient_color = {light_color.x, light_color.y, light_color.z};
                        m_light_env.ambient_intensity = std::max(light.ambient_intensity, 0.0f);
                        m_light_env.sky_luminance_rgb = m_light_env.ambient_color;
                    }
                    break;
                case rendering::LightSnapshot::Type::Point:
                    type_id = 1.0f;
                    break;
                case rendering::LightSnapshot::Type::Spot:
                    type_id = 2.0f;
                    {
                        const f32 inner_angle =
                            std::clamp(light.inner_cone_angle_deg, 0.0f, MAX_SPOT_INNER_HALF_ANGLE_DEG);
                        const f32 outer_angle =
                            std::clamp(std::max(light.outer_cone_angle_deg, inner_angle + MIN_SPOT_ANGLE_GAP_DEG),
                                       inner_angle + MIN_SPOT_ANGLE_GAP_DEG, MAX_SPOT_OUTER_HALF_ANGLE_DEG);
                        inner_cone_cos = spot_half_angle_cos(inner_angle);
                        outer_cone_cos = spot_half_angle_cos(outer_angle);
                    }
                    break;
            }

            push_light(light.position, light.direction, light_color, light.intensity, type_id, light.range,
                       inner_cone_cos, outer_cone_cos);
        }
    }

    void RenderSystem::submit_lighting(graphics::DeviceContext& context) const {
        const auto& packed = m_light_env.lights;
        constexpr u16 max = rendering::uniforms::MAX_LIGHTS;

        context.set_uniform(std::string(rendering::uniforms::LIGHTS_POS_TYPE), packed.pos_type.data(), max,
                            graphics::UniformType::Vec4);
        context.set_uniform(std::string(rendering::uniforms::LIGHTS_DIR_RANGE), packed.dir_range.data(), max,
                            graphics::UniformType::Vec4);
        context.set_uniform(std::string(rendering::uniforms::LIGHTS_COLOR_INT), packed.color_int.data(), max,
                            graphics::UniformType::Vec4);
        context.set_uniform(std::string(rendering::uniforms::LIGHTS_CONE), packed.cone.data(), max,
                            graphics::UniformType::Vec4);

        const Vector4 count{static_cast<f32>(packed.count), 0.0f, 0.0f, 0.0f};
        context.set_uniform(std::string(rendering::uniforms::LIGHTS_COUNT), &count, 1, graphics::UniformType::Vec4);

        const Vector4 ambient_color{m_light_env.ambient_color.x, m_light_env.ambient_color.y,
                                    m_light_env.ambient_color.z, m_light_env.ambient_intensity};
        context.set_uniform(std::string(rendering::uniforms::AMBIENT_COLOR), &ambient_color, 1,
                            graphics::UniformType::Vec4);

        const Vector4 ground_color{m_light_env.ground_color.x, m_light_env.ground_color.y, m_light_env.ground_color.z,
                                   m_light_env.exposure};
        context.set_uniform(std::string(rendering::uniforms::GROUND_COLOR), &ground_color, 1,
                            graphics::UniformType::Vec4);

        const Vector4 env_sky_color{m_light_env.sky_luminance_rgb.x, m_light_env.sky_luminance_rgb.y,
                                    m_light_env.sky_luminance_rgb.z, m_light_env.has_sky ? 1.0f : 0.0f};
        context.set_uniform(std::string(rendering::uniforms::ENV_SKY_COLOR), &env_sky_color, 1,
                            graphics::UniformType::Vec4);

        const Vector4 env_sun_dir{m_light_env.sun_direction.x, m_light_env.sun_direction.y, m_light_env.sun_direction.z,
                                  m_light_env.has_sun ? 1.0f : 0.0f};
        context.set_uniform(std::string(rendering::uniforms::ENV_SUN_DIR), &env_sun_dir, 1,
                            graphics::UniformType::Vec4);

        const Vector4 env_sun_color{m_light_env.sun_luminance_rgb.x, m_light_env.sun_luminance_rgb.y,
                                    m_light_env.sun_luminance_rgb.z, 0.0f};
        context.set_uniform(std::string(rendering::uniforms::ENV_SUN_COLOR), &env_sun_color, 1,
                            graphics::UniformType::Vec4);
    }

    void RenderSystem::render(const rendering::RenderScene& scene, graphics::DeviceContext& context, const u32 view_id,
                              rendering::Viewport* viewport) {
        if (!scene.active) {
            return;
        }

        m_render_queue.clear();

        if (scene.primary_camera.has_value()) {
            setup_camera(*scene.primary_camera, viewport);
        }
        collect_lights(scene);

        if (!viewport || !viewport->has_camera()) {
            STAR_LOG_WARN(LogCategory::Rendering, "No active camera found in scene '{}'", scene.name);
            return;
        }
        collect_renderables(scene, viewport);

        m_render_queue.sort();

        execute_render_queue(context, view_id, viewport);
    }

    void RenderSystem::setup_camera(const rendering::CameraSnapshot& camera, rendering::Viewport* viewport) const {
        if (!viewport) {
            return;
        }

        components::Camera cam;
        cam.fov_y = camera.fov_y;
        cam.near_plane = camera.near_plane;
        cam.far_plane = camera.far_plane;
        cam.aspect_ratio = camera.aspect_ratio;

        components::Transform xf;
        xf.position = camera.position;
        xf.rotation = camera.rotation;
        xf.scale = camera.scale;

        viewport->set_camera(cam, xf);

        STAR_LOG_TRACE(LogCategory::Rendering, "Camera setup: pos({}, {}, {})", xf.position.x, xf.position.y,
                       xf.position.z);
    }

    void RenderSystem::collect_renderables(const rendering::RenderScene& scene, const rendering::Viewport* viewport) {
        if (!viewport) {
            return;
        }

        const auto& camera_position = viewport->camera_position();
        const rendering::Frustum frustum =
            rendering::Frustum::extract(viewport->projection_matrix() * viewport->view_matrix());

        for (const auto& item : scene.renderables) {
            if (!item.mesh.is_valid()) {
                continue;
            }

            rendering::DrawCall draw_call;
            draw_call.model_matrix = item.model_matrix;
            draw_call.mesh = item.mesh;
            draw_call.material = item.material;
            draw_call.layer = item.layer;
            draw_call.parameters = item.parameters;

            if (const auto* mesh = m_resource_manager.get_mesh(item.mesh)) {
                if (!frustum.intersects_aabb_world(mesh->bounds, draw_call.model_matrix))
                    continue;
            }

            draw_call.distance_sq = camera_position.distance_squared_to(item.world_position);
            draw_call.is_transparent = item.is_transparent;

            m_render_queue.submit(draw_call);
        }
    }

    void RenderSystem::execute_render_queue(graphics::DeviceContext& context, const u32 view_id,
                                            const rendering::Viewport* viewport) const {
        if (viewport) {
            viewport->bind(context, view_id);
        }

        Vector4 cam_pos{};
        const bool has_camera = viewport != nullptr;
        if (has_camera) {
            const Vector3& cp = viewport->camera_position();
            cam_pos = {cp.x, cp.y, cp.z, 0.0f};
        }

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
                    submit_material(context, *material, command.parameters, m_resource_manager);

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
                submit_lighting(context);
                if (has_camera) {
                    context.set_uniform(std::string(rendering::uniforms::CAM_POS), &cam_pos, 1,
                                        graphics::UniformType::Vec4);
                }
                context.submit(view_id, shader_handle);
            } else {
                STAR_LOG_WARN(LogCategory::Rendering, "No valid shader available for rendering");
            }

            STAR_LOG_TRACE(LogCategory::Rendering, "Rendered opaque object (dist_sq: {:.2f})", command.distance_sq);
        }

        // TODO: transparent pass — sort back-to-front, enable blending per pipeline state
    }
} // namespace star::systems
