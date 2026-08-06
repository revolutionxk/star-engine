#pragma once
#include <string_view>

#include "star/core/types.hpp"
#include "star/graphics/device_context.hpp"

namespace star::rendering::uniforms {
    inline constexpr std::string_view LIGHT_DIR = "u_lightDir";
    inline constexpr std::string_view LIGHT_COLOR = "u_lightColor";
    inline constexpr std::string_view AMBIENT_COLOR = "u_ambientColor";
    inline constexpr std::string_view GROUND_COLOR = "u_groundColor";
    inline constexpr std::string_view ENV_SKY_COLOR = "u_envSkyColor";
    inline constexpr std::string_view ENV_SUN_DIR = "u_envSunDir";
    inline constexpr std::string_view ENV_SUN_COLOR = "u_envSunColor";

    inline constexpr std::string_view LIGHTS_POS_TYPE = "u_lightsPosType";
    inline constexpr std::string_view LIGHTS_DIR_RANGE = "u_lightsDirRange";
    inline constexpr std::string_view LIGHTS_COLOR_INT = "u_lightsColorInt";
    inline constexpr std::string_view LIGHTS_CONE = "u_lightsCone";
    inline constexpr std::string_view LIGHTS_COUNT = "u_lightsCount";
    inline constexpr u16 MAX_LIGHTS = 16;

    inline constexpr std::string_view BASE_COLOR = "u_baseColor";
    inline constexpr std::string_view MATERIAL_PARAMS = "u_materialParams";
    inline constexpr std::string_view EMISSIVE_COLOR = "u_emissive";
    inline constexpr std::string_view ALBEDO_TEXTURE = "u_albedoTexture";
    inline constexpr std::string_view TEX_FLAGS = "u_texFlags";
    inline constexpr std::string_view IBL_PARAMS = "u_iblParams";
    inline constexpr std::string_view IBL_PARAMS2 = "u_iblParams2";

    inline constexpr std::string_view CAM_POS = "u_camPos";

    inline constexpr std::string_view CASCADE_VIEW_PROJ = "u_cascadeViewProj";
    inline constexpr std::string_view CASCADE_SPLITS = "u_cascadeSplits";
    inline constexpr std::string_view CASCADE_OFFSETS = "u_cascadeOffsets";
    inline constexpr std::string_view SHADOW_PARAMS = "u_shadowParams";
    inline constexpr std::string_view SHADOW_PARAMS2 = "u_shadowParams2";

    inline constexpr std::string_view CUR_VIEW_PROJ_NJ = "u_curViewProjNJ";
    inline constexpr std::string_view PREV_VIEW_PROJ = "u_prevViewProj";
    inline constexpr std::string_view PREV_MODEL = "u_prevModel";

    inline constexpr std::string_view SAMPLER_ALBEDO = "s_texColor";
    inline constexpr std::string_view SAMPLER_NORMAL = "s_texNormal";
    inline constexpr std::string_view SAMPLER_MR = "s_texMetallicRoughness";
    inline constexpr std::string_view SAMPLER_EMISSIVE = "s_texEmissive";
    inline constexpr std::string_view SAMPLER_SHADOW = "s_shadowMap";
    inline constexpr std::string_view SAMPLER_SPECULAR_ENV = "s_specularEnv";
    inline constexpr std::string_view SAMPLER_IRRADIANCE_ENV = "s_irradianceEnv";
    inline constexpr std::string_view SAMPLER_BRDF_LUT = "s_brdfLut";

    inline constexpr u8 STAGE_ALBEDO = 0;
    inline constexpr u8 STAGE_NORMAL = 1;
    inline constexpr u8 STAGE_MR = 2;
    inline constexpr u8 STAGE_EMISSIVE = 3;
    inline constexpr u8 STAGE_SHADOW = 4;
    inline constexpr u8 STAGE_SPECULAR_ENV = 5;
    inline constexpr u8 STAGE_IRRADIANCE_ENV = 6;
    inline constexpr u8 STAGE_BRDF_LUT = 7;

    struct SceneUniforms {
        graphics::UniformId ambient_color;
        graphics::UniformId ground_color;
        graphics::UniformId env_sky_color;
        graphics::UniformId env_sun_dir;
        graphics::UniformId env_sun_color;

        graphics::UniformId lights_pos_type;
        graphics::UniformId lights_dir_range;
        graphics::UniformId lights_color_int;
        graphics::UniformId lights_cone;
        graphics::UniformId lights_count;

        graphics::UniformId base_color;
        graphics::UniformId material_params;
        graphics::UniformId emissive_color;
        graphics::UniformId tex_flags;
        graphics::UniformId ibl_params;
        graphics::UniformId ibl_params2;

        graphics::UniformId cam_pos;
        graphics::UniformId cascade_view_proj;
        graphics::UniformId cascade_splits;
        graphics::UniformId cascade_offsets;
        graphics::UniformId shadow_params;
        graphics::UniformId shadow_params2;

        graphics::UniformId cur_view_proj_nj;
        graphics::UniformId prev_view_proj;
        graphics::UniformId prev_model;

        graphics::UniformId sampler_albedo;
        graphics::UniformId sampler_normal;
        graphics::UniformId sampler_mr;
        graphics::UniformId sampler_emissive;
        graphics::UniformId sampler_shadow;
        graphics::UniformId sampler_specular_env;
        graphics::UniformId sampler_irradiance_env;
        graphics::UniformId sampler_brdf_lut;

        void resolve(graphics::DeviceContext& context);

        [[nodiscard]] bool is_resolved() const noexcept {
            return cam_pos.is_valid();
        }
    };
} // namespace star::rendering::uniforms
