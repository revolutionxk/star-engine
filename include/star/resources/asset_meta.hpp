#pragma once

#include <filesystem>

#include "star/core/uuid.hpp"

namespace star::resources {
    [[nodiscard]] std::filesystem::path meta_path_for(const std::filesystem::path& asset_path);

    [[nodiscard]] UUID read_asset_uuid(const std::filesystem::path& asset_path);

    bool write_asset_uuid(const std::filesystem::path& asset_path, const UUID& uuid);

    [[nodiscard]] UUID ensure_asset_uuid(const std::filesystem::path& asset_path);
} // namespace star::resources
