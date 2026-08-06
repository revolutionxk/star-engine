#include "star/resources/material_asset.hpp"

#include "star/math/math.hpp"
#include "star/resources/material/material.hpp"
#include "star/resources/resource_manager.hpp"

namespace star::resources {
    namespace detail {
        constexpr const char* MATERIAL_KEY = "star_material";

        nlohmann::json to_json(const Vector4& v) {
            return {{"x", v.x}, {"y", v.y}, {"z", v.z}, {"w", v.w}};
        }

        Vector4 vec4_or(const nlohmann::json& j, Vector4 fallback) {
            if (j.is_object()) {
                fallback.x = j.value("x", fallback.x);
                fallback.y = j.value("y", fallback.y);
                fallback.z = j.value("z", fallback.z);
                fallback.w = j.value("w", fallback.w);
            }
            return fallback;
        }
    } // namespace detail

    nlohmann::json material_to_json(const Material& material, const ResourceManager& resources) {
        nlohmann::json body;
        body["shader"] = resources.shader_name(material.shader);
        body["albedo_color"] = detail::to_json(material.albedo_color);
        body["emissive_color"] = detail::to_json(material.emissive_color);
        body["metallic"] = material.metallic;
        body["roughness"] = material.roughness;
        body["albedo_texture"] = resources.texture_name(material.albedo_texture);
        body["normal_texture"] = resources.texture_name(material.normal_texture);
        body["metallic_roughness_texture"] = resources.texture_name(material.metallic_roughness_texture);
        body["emissive_texture"] = resources.texture_name(material.emissive_texture);

        nlohmann::json document;
        document[detail::MATERIAL_KEY] = std::move(body);
        return document;
    }

    void material_from_json(const nlohmann::json& document, Material& material, ResourceManager& resources) {
        const nlohmann::json& body = document.contains(detail::MATERIAL_KEY) ? document[detail::MATERIAL_KEY] : document;

        const auto shader = resources.shader_by_name(body.value("shader", std::string{}));
        material.shader = shader.is_valid() ? shader : resources.default_shader();

        if (body.contains("albedo_color"))
            material.albedo_color = detail::vec4_or(body["albedo_color"], material.albedo_color);
        if (body.contains("emissive_color"))
            material.emissive_color = detail::vec4_or(body["emissive_color"], material.emissive_color);
        material.metallic = body.value("metallic", material.metallic);
        material.roughness = body.value("roughness", material.roughness);

        if (const auto texture = body.value("albedo_texture", std::string{}); !texture.empty())
            material.albedo_texture = resources.get_or_load_texture(texture);
        if (const auto texture = body.value("normal_texture", std::string{}); !texture.empty())
            material.normal_texture = resources.get_or_load_texture(texture);
        if (const auto texture = body.value("metallic_roughness_texture", std::string{}); !texture.empty())
            material.metallic_roughness_texture = resources.get_or_load_texture(texture);
        if (const auto texture = body.value("emissive_texture", std::string{}); !texture.empty())
            material.emissive_texture = resources.get_or_load_texture(texture);
    }
} // namespace star::resources
