#include "icon_registry.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>

#include <lunasvg.h>

#include "star/core/logger.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/texture/texture.hpp"

namespace star::editor {
    u64 IconRegistry::icon(const std::string& name) {
        if (const auto it = m_cache.find(name); it != m_cache.end())
            return it->second;

        m_cache[name] = 0;

        const std::filesystem::path path = std::filesystem::path("assets/icons/lucide") / (name + ".svg");
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            STAR_LOG_WARN(LogCategory::Editor, "Icon SVG not found: {}", path.string());
            return 0;
        }
        std::stringstream ss;
        ss << file.rdbuf();
        std::string svg = ss.str();

        for (std::size_t p = svg.find("currentColor"); p != std::string::npos; p = svg.find("currentColor", p))
            svg.replace(p, 12, "#ffffff");

        const auto document = lunasvg::Document::loadFromData(svg);
        if (!document) {
            STAR_LOG_WARN(LogCategory::Editor, "Failed to parse icon SVG: {}", name);
            return 0;
        }

        lunasvg::Bitmap bitmap = document->renderToBitmap(RASTER_SIZE, RASTER_SIZE);
        if (bitmap.isNull() || bitmap.width() == 0 || bitmap.height() == 0)
            return 0;
        bitmap.convertToRGBA();

        auto texture = std::make_unique<resources::Texture>();
        texture->desc.width = static_cast<u32>(bitmap.width());
        texture->desc.height = static_cast<u32>(bitmap.height());
        texture->desc.format = resources::TextureFormat::RGBA8;
        texture->desc.generate_mipmaps = false;
        const auto* pixels = bitmap.data();
        const std::size_t bytes = static_cast<std::size_t>(bitmap.width()) * bitmap.height() * 4;
        texture->data.assign(pixels, pixels + bytes);

        const auto handle = m_resources.create_texture("__icon_" + name, std::move(texture));
        const auto* res = m_resources.get_texture(handle);
        const u64 id = res ? static_cast<u64>(res->handle.id) : 0;
        m_cache[name] = id;
        return id;
    }
} // namespace star::editor
