#include "star/resources/asset_meta.hpp"

#include <fstream>

#include <nlohmann/json.hpp>

#include "star/core/logger.hpp"

namespace star::resources {
    std::filesystem::path meta_path_for(const std::filesystem::path& asset_path) {
        std::filesystem::path meta = asset_path;
        meta += ".meta";
        return meta;
    }

    Uuid read_asset_uuid(const std::filesystem::path& asset_path) {
        const std::filesystem::path meta = meta_path_for(asset_path);

        std::ifstream in(meta);
        if (!in) {
            return {};
        }

        nlohmann::json document;
        try {
            in >> document;
        } catch (const std::exception& e) {
            STAR_LOG_WARN(LogCategory::Resources, "Malformed asset meta '{}': {}", meta.string(), e.what());
            return {};
        }

        const auto text = document.value("uuid", std::string{});
        const auto parsed = Uuid::parse(text);
        if (!parsed.has_value()) {
            STAR_LOG_WARN(LogCategory::Resources, "Asset meta '{}' has no usable uuid", meta.string());
            return {};
        }
        return *parsed;
    }

    bool write_asset_uuid(const std::filesystem::path& asset_path, const Uuid& uuid) {
        const std::filesystem::path meta = meta_path_for(asset_path);

        std::error_code ec;
        if (!meta.parent_path().empty()) {
            std::filesystem::create_directories(meta.parent_path(), ec);
        }

        std::ofstream out(meta);
        if (!out) {
            STAR_LOG_ERROR(LogCategory::Resources, "Failed to write asset meta: {}", meta.string());
            return false;
        }

        nlohmann::json document;
        document["uuid"] = uuid.to_string();
        document["version"] = 1;
        out << document.dump(4);
        return true;
    }

    Uuid ensure_asset_uuid(const std::filesystem::path& asset_path) {
        if (const Uuid existing = read_asset_uuid(asset_path); existing.is_valid()) {
            return existing;
        }

        const Uuid created = Uuid::generate();
        write_asset_uuid(asset_path, created);
        return created;
    }
} // namespace star::resources
