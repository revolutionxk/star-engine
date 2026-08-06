#include <string>

#include "star/core/reflection/reflect.hpp"
#include "star/core/uuid.hpp"
#include "star/ecs/serialization.hpp"
#include "star/rendering/components/mesh_renderer.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/resource_resolver.hpp"

namespace star::components::detail {
    void write_uuid(nlohmann::json& j, const char* key, const UUID& uuid) {
        if (uuid.is_valid()) {
            j[key] = uuid.to_string();
        }
    }

    UUID read_uuid(const nlohmann::json& j, const char* key) {
        if (!j.contains(key)) {
            return {};
        }
        const auto parsed = UUID::parse(j.value(key, std::string{}));
        return parsed.value_or(UUID{});
    }
} // namespace star::components::detail

namespace star::components {
    nlohmann::json to_json(const MeshRenderer& value) {
        nlohmann::json j;
        reflection::for_each_field(value, ecs::JsonWriteVisitor{j});

        if (const auto* rm = resources::ResourceResolver::active()) {
            j["Mesh"] = rm->mesh_name(value.mesh);
            j["Material"] = rm->material_name(value.material);
            detail::write_uuid(j, "MeshUuid", rm->mesh_uuid(value.mesh));
            detail::write_uuid(j, "MaterialUuid", rm->material_uuid(value.material));
        }
        return j;
    }

    void from_json(const nlohmann::json& j, MeshRenderer& value) {
        reflection::for_each_field(value, ecs::JsonReadVisitor{j});

        auto* rm = resources::ResourceResolver::active();
        if (!rm) {
            return;
        }

        if (const UUID mesh_uuid = detail::read_uuid(j, "MeshUuid"); mesh_uuid.is_valid()) {
            value.mesh = rm->mesh_by_uuid(mesh_uuid);
        }
        if (!value.mesh.is_valid() && j.contains("Mesh")) {
            value.mesh = rm->mesh_by_name(j.value("Mesh", std::string{}));
        }

        if (const UUID material_uuid = detail::read_uuid(j, "MaterialUuid"); material_uuid.is_valid()) {
            value.material = rm->material_by_uuid(material_uuid);
        }
        if (!value.material.is_valid() && j.contains("Material")) {
            value.material = rm->get_or_load_material(j.value("Material", std::string{}));
        }
    }
} // namespace star::components
