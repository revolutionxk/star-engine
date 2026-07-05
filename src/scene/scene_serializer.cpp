#include "star/scene/scene_serializer.hpp"

#include <fstream>
#include <functional>
#include <vector>

#include "star/core/common.hpp"
#include "star/ecs/component_registry.hpp"
#include "star/scene/scene.hpp"

namespace star::scene {
    nlohmann::json serialize_scene(const Scene& scene) {
        std::function<nlohmann::json(flecs::entity)> serialize_node = [&](const flecs::entity entity) {
            nlohmann::json node;
            node["name"] = entity.name().c_str();
            node["components"] = ecs::serialize_entity(entity);

            nlohmann::json children = nlohmann::json::array();
            entity.children([&](const flecs::entity child) { children.push_back(serialize_node(child)); });
            if (!children.empty())
                node["children"] = std::move(children);

            return node;
        };

        nlohmann::json entities = nlohmann::json::array();
        scene.root().children([&](const flecs::entity entity) {
            try {
                entities.push_back(serialize_node(entity));
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

    void load_scene(const Scene& scene, const nlohmann::json& document) {
        std::vector<flecs::entity> existing;
        scene.root().children([&](const flecs::entity entity) { existing.push_back(entity); });
        for (const auto& entity : existing) {
            entity.destruct();
        }

        if (!document.contains("entities")) {
            return;
        }

        std::function<void(const nlohmann::json&, const Entity*)> load_node = [&](const nlohmann::json& node,
                                                                                  const Entity* parent) {
            const auto name = node.value("name", std::string{});
            Entity entity = scene.create_entity(name);
            if (parent)
                entity.child_of(*parent);
            if (node.contains("components"))
                ecs::deserialize_entity(entity.raw(), node.at("components"));
            if (node.contains("children"))
                for (const auto& child : node.at("children"))
                    load_node(child, &entity);
        };

        for (const auto& node : document.at("entities"))
            load_node(node, nullptr);
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
