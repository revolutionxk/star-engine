#pragma once
#include <string_view>

namespace star::rendering::uniforms {
    inline constexpr std::string_view LIGHT_DIR = "u_lightDir";
    inline constexpr std::string_view LIGHT_COLOR = "u_lightColor";
    inline constexpr std::string_view AMBIENT_COLOR = "u_ambientColor";

    inline constexpr std::string_view BASE_COLOR = "u_baseColor";
    inline constexpr std::string_view MATERIAL_PARAMS = "u_materialParams";
    inline constexpr std::string_view ALBEDO_TEXTURE = "u_albedoTexture";

    inline constexpr std::string_view CAM_POS = "u_camPos";

    inline constexpr u8 STAGE_ALBEDO = 0;
    inline constexpr u8 STAGE_NORMAL = 1;
    inline constexpr u8 STAGE_METALLIC = 2;
    inline constexpr u8 STAGE_ROUGHNESS = 3;
} // namespace star::rendering::uniforms
