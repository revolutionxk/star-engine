#include "star/scene/scene_serializer.hpp"

#include <fstream>
#include <vector>

#include "star/core/common.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/scene/scene.hpp"

namespace star::scene {
    nlohmann::json serialize_scene(const Scene& scene) {
        nlohmann::json entities = nlohmann::json::array();

        scene.root().children([&](const flecs::entity entity) {
            try {
                nlohmann::json entity_json;
                entity_json["name"] = entity.name().c_str();
                entity_json["components"] = ecs::serialize_entity(entity);
                entities.push_back(std::move(entity_json));
            } catch (const std::exception& e) {
                STAR_LOG_WARN(LogCategory::Scene, "Skipped entity '{}' while saving: {}", entity.name().c_str(),
                              e.what());
            }
        });

        nlohmann::json document;
        document["scene"] = scene.name();
        document["entities"] = std::move(entities);
        return document;
    }

    void load_scene(Scene& scene, const nlohmann::json& document) {
        std::vector<flecs::entity> existing;
        scene.root().children([&](const flecs::entity entity) { existing.push_back(entity); });
        for (const auto& entity : existing) {
            entity.destruct();
        }

        if (!document.contains("entities")) {
            return;
        }

        for (const auto& entity_json : document.at("entities")) {
            const auto name = entity_json.value("name", std::string{});
            const auto entity = scene.create_entity(name);
            if (entity_json.contains("components")) {
                ecs::deserialize_entity(entity.raw(), entity_json.at("components"));
            }
        }
    }

    bool save_scene_to_file(Scene& scene, const std::filesystem::path& path) {
        try {
            std::ofstream file(path);
            if (!file) {
                STAR_LOG_ERROR(LogCategory::Scene, "Could not open '{}' for writing", path.string());
                return false;
            }
            file << serialize_scene(scene).dump(2);
            STAR_LOG_INFO(LogCategory::Scene, "Saved scene to '{}'", path.string());
            return true;
        } catch (const std::exception& e) {
            STAR_LOG_ERROR(LogCategory::Scene, "Failed to save scene: {}", e.what());
            return false;
        }
    }

    bool load_scene_from_file(Scene& scene, const std::filesystem::path& path) {
        try {
            std::ifstream file(path);
            if (!file) {
                STAR_LOG_ERROR(LogCategory::Scene, "Could not open '{}' for reading", path.string());
                return false;
            }
            nlohmann::json document;
            file >> document;
            load_scene(scene, document);
            STAR_LOG_INFO(LogCategory::Scene, "Loaded scene from '{}'", path.string());
            return true;
        } catch (const std::exception& e) {
            STAR_LOG_ERROR(LogCategory::Scene, "Failed to load scene: {}", e.what());
            return false;
        }
    }
} // namespace star::scene
