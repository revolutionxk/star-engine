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

using namespace star;

namespace {
    constexpr f32 MAX_SPOT_INNER_HALF_ANGLE_DEG = 89.0f;
    constexpr f32 MAX_SPOT_OUTER_HALF_ANGLE_DEG = 89.9f;
    constexpr f32 MIN_SPOT_ANGLE_GAP_DEG = 0.1f;

    [[nodiscard]] f32 spot_half_angle_cos(const f32 angle_deg) {
        return std::cos(radians(angle_deg));
    }

    enum MaterialOverride : u32 {
        OverrideNone = 0,
        OverrideBaseColor = 1u << 0,
        OverrideMaterialParams = 1u << 1,
        OverrideEmissive = 1u << 2,
    };

    [[nodiscard]] u32 collect_overrides(const std::vector<rendering::MaterialProperty>& params) {
        u32 mask = OverrideNone;
        for (const auto& p : params) {
            if (p.name == rendering::uniforms::BASE_COLOR)
                mask |= OverrideBaseColor;
            else if (p.name == rendering::uniforms::MATERIAL_PARAMS)
                mask |= OverrideMaterialParams;
            else if (p.name == rendering::uniforms::EMISSIVE_COLOR)
                mask |= OverrideEmissive;
        }
        return mask;
    }

    void upload_property(graphics::DeviceContext& ctx, const rendering::MaterialProperty& prop) {
        std::visit(
            [&]<typename Value>(const Value& v) {
                using T = std::decay_t<Value>;

                if constexpr (std::is_same_v<T, float>) {
                    const Vector4 padded{v, 0.0f, 0.0f, 0.0f};
                    ctx.set_uniform(ctx.uniform(prop.name, graphics::UniformType::Vec4), &padded);

                } else if constexpr (std::is_same_v<T, Vector2>) {
                    const Vector4 padded{v.x, v.y, 0.0f, 0.0f};
                    ctx.set_uniform(ctx.uniform(prop.name, graphics::UniformType::Vec4), &padded);

                } else if constexpr (std::is_same_v<T, Vector3>) {
                    const Vector4 padded{v.x, v.y, v.z, 0.0f};
                    ctx.set_uniform(ctx.uniform(prop.name, graphics::UniformType::Vec4), &padded);

                } else if constexpr (std::is_same_v<T, Vector4>) {
                    ctx.set_uniform(ctx.uniform(prop.name, graphics::UniformType::Vec4), &v);

                } else if constexpr (std::is_same_v<T, Matrix4>) {
                    ctx.set_uniform(ctx.uniform(prop.name, graphics::UniformType::Mat4), &v);

                } else if constexpr (std::is_same_v<T, graphics::ResourceHandle<graphics::Texture>>) {
                    if (v.is_valid())
                        ctx.set_texture(ctx.uniform(prop.name, graphics::UniformType::Sampler), prop.texture_stage, v);
                }
            },
            prop.value);
    }

    void submit_material(graphics::DeviceContext& ctx, const rendering::uniforms::SceneUniforms& ids,
                         const resources::Material& mat, const std::vector<rendering::MaterialProperty>& params,
                         resources::ResourceManager& rm, const bool transparent) {
        graphics::PipelineState state = mat.pipeline_state;
        if (transparent) {
            state.depth_write = false;
            if (state.blend_mode == graphics::BlendMode::Opaque) {
                state.blend_mode = graphics::BlendMode::AlphaBlend;
            }
        }
        ctx.set_pipeline_state(state);

        const u32 overridden = collect_overrides(params);

        if (!(overridden & OverrideBaseColor))
            ctx.set_uniform(ids.base_color, &mat.albedo_color);

        if (!(overridden & OverrideMaterialParams)) {
            const Vector4 pbr_params{mat.metallic, mat.roughness, 0.0f, 0.0f};
            ctx.set_uniform(ids.material_params, &pbr_params);
        }

        if (!(overridden & OverrideEmissive))
            ctx.set_uniform(ids.emissive_color, &mat.emissive_color);

        const auto bind_stage = [&](const graphics::UniformId sampler, const u8 stage,
                                    const graphics::ResourceHandle<resources::Texture>& handle) {
            if (handle.is_valid())
                if (const auto* tex = rm.get_texture(handle)) {
                    ctx.set_texture(sampler, stage, tex->handle);
                    return true;
                }
            if (const auto* white = rm.get_texture(rm.white_texture()))
                ctx.set_texture(sampler, stage, white->handle);
            return false;
        };

        const bool has_albedo =
            bind_stage(ids.sampler_albedo, rendering::uniforms::STAGE_ALBEDO, mat.albedo_texture);
        const bool has_normal =
            bind_stage(ids.sampler_normal, rendering::uniforms::STAGE_NORMAL, mat.normal_texture);
        const bool has_mr =
            bind_stage(ids.sampler_mr, rendering::uniforms::STAGE_MR, mat.metallic_roughness_texture);
        const bool has_emissive =
            bind_stage(ids.sampler_emissive, rendering::uniforms::STAGE_EMISSIVE, mat.emissive_texture);

        const Vector4 tex_flags{has_albedo ? 1.0f : 0.0f, has_normal ? 1.0f : 0.0f, has_mr ? 1.0f : 0.0f,
                                has_emissive ? 1.0f : 0.0f};
        ctx.set_uniform(ids.tex_flags, &tex_flags);

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
        const auto& ids = m_uniforms;
        constexpr u16 max = rendering::uniforms::MAX_LIGHTS;

        context.set_uniform(ids.lights_pos_type, packed.pos_type.data(), max);
        context.set_uniform(ids.lights_dir_range, packed.dir_range.data(), max);
        context.set_uniform(ids.lights_color_int, packed.color_int.data(), max);
        context.set_uniform(ids.lights_cone, packed.cone.data(), max);

        const Vector4 count{static_cast<f32>(packed.count), 0.0f, 0.0f, 0.0f};
        context.set_uniform(ids.lights_count, &count);

        const Vector4 ambient_color{m_light_env.ambient_color.x, m_light_env.ambient_color.y,
                                    m_light_env.ambient_color.z, m_light_env.ambient_intensity};
        context.set_uniform(ids.ambient_color, &ambient_color);

        const Vector4 ground_color{m_light_env.ground_color.x, m_light_env.ground_color.y, m_light_env.ground_color.z,
                                   m_light_env.exposure};
        context.set_uniform(ids.ground_color, &ground_color);

        const Vector4 env_sky_color{m_light_env.sky_luminance_rgb.x, m_light_env.sky_luminance_rgb.y,
                                    m_light_env.sky_luminance_rgb.z, m_light_env.has_sky ? 1.0f : 0.0f};
        context.set_uniform(ids.env_sky_color, &env_sky_color);

        const Vector4 env_sun_dir{m_light_env.sun_direction.x, m_light_env.sun_direction.y, m_light_env.sun_direction.z,
                                  m_light_env.has_sun ? 1.0f : 0.0f};
        context.set_uniform(ids.env_sun_dir, &env_sun_dir);

        const Vector4 env_sun_color{m_light_env.sun_luminance_rgb.x, m_light_env.sun_luminance_rgb.y,
                                    m_light_env.sun_luminance_rgb.z, 0.0f};
        context.set_uniform(ids.env_sun_color, &env_sun_color);

        const Vector4 shadow_params{m_shadow.enabled ? 1.0f : 0.0f, m_shadow.bias, m_shadow.texel_size,
                                    m_shadow.origin_bottom_left ? 1.0f : 0.0f};
        context.set_uniform(ids.shadow_params, &shadow_params);
        context.set_uniform(ids.light_view_proj, &m_shadow.light_view_proj);
        if (m_shadow.map.is_valid())
            context.set_texture(ids.sampler_shadow, rendering::uniforms::STAGE_SHADOW, m_shadow.map);

        const bool ibl_enabled = m_environment.map.is_valid();
        const Vector4 ibl_params{ibl_enabled ? 1.0f : 0.0f, m_environment.max_mip, m_environment.intensity, 0.0f};
        context.set_uniform(ids.ibl_params, &ibl_params);
        if (ibl_enabled)
            context.set_texture(ids.sampler_env, rendering::uniforms::STAGE_ENV, m_environment.map);
        else if (const auto* black = m_resource_manager.get_texture(m_resource_manager.black_texture()))
            context.set_texture(ids.sampler_env, rendering::uniforms::STAGE_ENV, black->handle);
    }

    void RenderSystem::render(const rendering::RenderScene& scene, graphics::DeviceContext& context, const u32 view_id,
                              rendering::Viewport* viewport) {
        if (!scene.active) {
            return;
        }

        m_render_queue.clear();

        if (!m_uniforms.is_resolved()) {
            m_uniforms.resolve(context);
        }

        collect_lights(scene);

        if (!viewport || !viewport->has_camera()) {
            STAR_LOG_WARN(LogCategory::Rendering, "No active camera found in scene '{}'", scene.name);
            return;
        }

        m_prev_models.swap(m_cur_models);
        m_cur_models.clear();

        collect_renderables(scene, viewport);

        m_render_queue.sort();

        execute_render_queue(context, view_id, viewport);
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
            draw_call.entity_id = item.entity_id;

            if (const auto* mesh = m_resource_manager.get_mesh(item.mesh)) {
                if (!frustum.intersects_aabb_world(mesh->bounds, draw_call.model_matrix))
                    continue;
            }

            draw_call.distance_sq = camera_position.distance_squared_to(item.world_position);

            draw_call.is_transparent = item.is_transparent;
            if (!draw_call.is_transparent && item.material.is_valid()) {
                if (const auto* material = m_resource_manager.get_material(item.material)) {
                    draw_call.is_transparent =
                        material->pipeline_state.blend_mode != graphics::BlendMode::Opaque;
                }
            }

            m_render_queue.submit(draw_call);
        }
    }

    void RenderSystem::execute_render_queue(graphics::DeviceContext& context, const u32 view_id,
                                            const rendering::Viewport* viewport) const {
        Vector4 cam_pos{};
        if (viewport) {
            viewport->bind(context, view_id);
            const Vector3& cp = viewport->camera_position();
            cam_pos = {cp.x, cp.y, cp.z, 0.0f};
        }

        submit_draw_calls(context, view_id, viewport, cam_pos, m_render_queue.opaque_commands(), false);
        submit_draw_calls(context, view_id, viewport, cam_pos, m_render_queue.transparent_commands(), true);
    }

    void RenderSystem::submit_draw_calls(graphics::DeviceContext& context, const u32 view_id,
                                         const rendering::Viewport* viewport, const Vector4& cam_pos,
                                         const std::vector<rendering::DrawCall>& commands,
                                         const bool transparent) const {
        for (const auto& command : commands) {
            if (!command.mesh.is_valid()) {
                continue;
            }

            const auto* mesh = m_resource_manager.get_mesh(command.mesh);
            if (!mesh) {
                STAR_LOG_WARN(LogCategory::Rendering, "Failed to get mesh from handle");
                continue;
            }

            context.set_transform(command.model_matrix);

            const auto pit = m_prev_models.find(command.entity_id);
            const Matrix4& prev_model = pit != m_prev_models.end() ? pit->second : command.model_matrix;
            context.set_uniform(m_uniforms.prev_model, &prev_model);
            m_cur_models[command.entity_id] = command.model_matrix;

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
                    submit_material(context, m_uniforms, *material, command.parameters, m_resource_manager,
                                    transparent);

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
                if (viewport) {
                    context.set_uniform(m_uniforms.cam_pos, &cam_pos);
                    context.set_uniform(m_uniforms.cur_view_proj_nj, &viewport->cur_view_proj());
                    context.set_uniform(m_uniforms.prev_view_proj, &viewport->prev_view_proj());
                }
                context.submit(view_id, shader_handle);
            } else {
                STAR_LOG_WARN(LogCategory::Rendering, "No valid shader available for rendering");
            }
        }
    }
} // namespace star::systems
