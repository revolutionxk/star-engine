#include <string>

#include "star/core/reflection/reflect.hpp"
#include "star/ecs/serialization.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/resource_resolver.hpp"

namespace star::components {
    nlohmann::json to_json(const MeshRenderer& value) {
        nlohmann::json j;
        reflection::for_each_field(value, ecs::JsonWriteVisitor{j});

        if (const auto* rm = resources::ResourceResolver::active()) {
            j["Mesh"] = rm->mesh_name(value.mesh);
            j["Material"] = rm->material_name(value.material);
        }
        return j;
    }

    void from_json(const nlohmann::json& j, MeshRenderer& value) {
        reflection::for_each_field(value, ecs::JsonReadVisitor{j});

        if (auto* rm = resources::ResourceResolver::active()) {
            if (j.contains("Mesh"))
                value.mesh = rm->mesh_by_name(j.value("Mesh", std::string{}));
            if (j.contains("Material"))
                value.material = rm->get_or_load_material(j.value("Material", std::string{}));
        }
    }
} // namespace star::components
