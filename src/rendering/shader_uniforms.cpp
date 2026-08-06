#include "star/rendering/shader_uniforms.hpp"

namespace star::rendering::uniforms {
    void SceneUniforms::resolve(graphics::DeviceContext& context) {
        using graphics::UniformType;

        const auto vec4 = [&](const std::string_view name, const u16 num = 1) {
            return context.uniform(name, UniformType::Vec4, num);
        };
        const auto mat4 = [&](const std::string_view name) {
            return context.uniform(name, UniformType::Mat4, 1);
        };
        const auto sampler = [&](const std::string_view name) {
            return context.uniform(name, UniformType::Sampler, 1);
        };

        ambient_color = vec4(AMBIENT_COLOR);
        ground_color = vec4(GROUND_COLOR);
        env_sky_color = vec4(ENV_SKY_COLOR);
        env_sun_dir = vec4(ENV_SUN_DIR);
        env_sun_color = vec4(ENV_SUN_COLOR);

        lights_pos_type = vec4(LIGHTS_POS_TYPE, MAX_LIGHTS);
        lights_dir_range = vec4(LIGHTS_DIR_RANGE, MAX_LIGHTS);
        lights_color_int = vec4(LIGHTS_COLOR_INT, MAX_LIGHTS);
        lights_cone = vec4(LIGHTS_CONE, MAX_LIGHTS);
        lights_count = vec4(LIGHTS_COUNT);

        base_color = vec4(BASE_COLOR);
        material_params = vec4(MATERIAL_PARAMS);
        emissive_color = vec4(EMISSIVE_COLOR);
        tex_flags = vec4(TEX_FLAGS);
        ibl_params = vec4(IBL_PARAMS);

        light_view_proj = mat4(LIGHT_VIEW_PROJ);
        shadow_params = vec4(SHADOW_PARAMS);

        cur_view_proj_nj = mat4(CUR_VIEW_PROJ_NJ);
        prev_view_proj = mat4(PREV_VIEW_PROJ);
        prev_model = mat4(PREV_MODEL);

        sampler_albedo = sampler(SAMPLER_ALBEDO);
        sampler_normal = sampler(SAMPLER_NORMAL);
        sampler_mr = sampler(SAMPLER_MR);
        sampler_emissive = sampler(SAMPLER_EMISSIVE);
        sampler_shadow = sampler(SAMPLER_SHADOW);
        sampler_env = sampler(SAMPLER_ENV);

        cam_pos = vec4(CAM_POS);
    }
} // namespace star::rendering::uniforms
