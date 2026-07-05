#pragma once

#include <filesystem>

#include <nlohmann/json.hpp>

namespace star::scene {
    class Scene;
    
    [[nodiscard]] nlohmann::json serialize_scene(const Scene& scene);
    void load_scene(Scene& scene, const nlohmann::json& document);

    [[nodiscard]] bool save_scene_to_file(Scene& scene, const std::filesystem::path& path);
    [[nodiscard]] bool load_scene_from_file(Scene& scene, const std::filesystem::path& path);
} // namespace star::scene
