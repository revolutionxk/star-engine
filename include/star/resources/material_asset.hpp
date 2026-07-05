#pragma once

#include <nlohmann/json.hpp>

namespace star::resources {
    struct Material;
    class ResourceManager;
    
    [[nodiscard]] nlohmann::json material_to_json(const Material& material, const ResourceManager& resources);
    void material_from_json(const nlohmann::json& document, Material& material, ResourceManager& resources);
} // namespace star::resources
