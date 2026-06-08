#pragma once
#include <string_view>

#include "star/core/types.hpp"

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

    inline constexpr std::string_view CAM_POS = "u_camPos";

    inline constexpr u8 STAGE_ALBEDO = 0;
    inline constexpr u8 STAGE_NORMAL = 1;
    inline constexpr u8 STAGE_METALLIC = 2;
    inline constexpr u8 STAGE_ROUGHNESS = 3;
} // namespace star::rendering::uniforms
