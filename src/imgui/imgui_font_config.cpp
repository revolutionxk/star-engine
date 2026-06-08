#include "star/imgui/imgui_font_config.hpp"

#include <imgui.h>

#ifdef IMGUI_ENABLE_FREETYPE
    #include <imgui_freetype.h>
#endif

#include "star/core/logger.hpp"

namespace star::platform {
    FontGlyphRange FontGlyphRange::default_range() {
        return {"Default", ImGui::GetIO().Fonts->GetGlyphRangesDefault()};
    }

    FontGlyphRange FontGlyphRange::latin() {
        return {"Latin", ImGui::GetIO().Fonts->GetGlyphRangesDefault()};
    }

    FontGlyphRange FontGlyphRange::cyrillic() {
        return {"Cyrillic", ImGui::GetIO().Fonts->GetGlyphRangesCyrillic()};
    }

    FontGlyphRange FontGlyphRange::japanese() {
        return {"Japanese", ImGui::GetIO().Fonts->GetGlyphRangesJapanese()};
    }

    FontGlyphRange FontGlyphRange::chinese_full() {
        return {"ChineseFull", ImGui::GetIO().Fonts->GetGlyphRangesChineseFull()};
    }

    FontGlyphRange FontGlyphRange::chinese_simplified_common() {
        return {"ChineseSimplifiedCommon", ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon()};
    }

    FontGlyphRange FontGlyphRange::korean() {
        return {"Korean", ImGui::GetIO().Fonts->GetGlyphRangesKorean()};
    }

    FontGlyphRange FontGlyphRange::thai() {
        return {"Thai", ImGui::GetIO().Fonts->GetGlyphRangesThai()};
    }

    FontGlyphRange FontGlyphRange::vietnamese() {
        return {"Vietnamese", ImGui::GetIO().Fonts->GetGlyphRangesVietnamese()};
    }

    ImFont* ImGuiFontManager::load_font(const FontConfiguration& config) {
        if (!std::filesystem::exists(config.font_path)) {
            STAR_LOG_ERROR(LogCategory::Platform, "Font file not found: {}", config.font_path.string());
            return nullptr;
        }

        const auto& io = ImGui::GetIO();

        ImFontConfig imgui_config;
        imgui_config.MergeMode = config.merge_mode;
        imgui_config.OversampleH = config.oversample_h;
        imgui_config.OversampleV = config.oversample_v;
        imgui_config.PixelSnapH = config.pixel_snap;

#ifdef IMGUI_ENABLE_FREETYPE
        if (config.freetype_enabled) {
            io.Fonts->FontLoader = ImGuiFreeType::GetFontLoader();
            io.Fonts->FontLoaderFlags =
                config.freetype_flags != 0 ? config.freetype_flags : ImGuiFreeTypeBuilderFlags_LightHinting;

            STAR_LOG_INFO(LogCategory::Platform, "Using FreeType for font rasterization");
        }
#endif

        ImFont* font = nullptr;

        if (config.glyph_ranges.empty()) {
            font = io.Fonts->AddFontFromFileTTF(config.font_path.string().c_str(), config.size_pixels, &imgui_config,
                                                io.Fonts->GetGlyphRangesDefault());
        } else if (config.glyph_ranges.size() == 1) {
            font = io.Fonts->AddFontFromFileTTF(config.font_path.string().c_str(), config.size_pixels, &imgui_config,
                                                static_cast<const ImWchar*>(config.glyph_ranges[0].ranges));
        } else {
            imgui_config.MergeMode = false;
            font = io.Fonts->AddFontFromFileTTF(config.font_path.string().c_str(), config.size_pixels, &imgui_config,
                                                static_cast<const ImWchar*>(config.glyph_ranges[0].ranges));

            imgui_config.MergeMode = true;
            for (size_t i = 1; i < config.glyph_ranges.size(); ++i) {
                io.Fonts->AddFontFromFileTTF(config.font_path.string().c_str(), config.size_pixels, &imgui_config,
                                             static_cast<const ImWchar*>(config.glyph_ranges[i].ranges));
            }
        }

        if (font) {
            STAR_LOG_INFO(LogCategory::Platform, "Font loaded: {} ({}px)", config.font_path.filename().string(),
                          config.size_pixels);
        } else {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to load font: {}", config.font_path.string());
        }

        return font;
    }

    ImFont* ImGuiFontManager::load_default_font(const f32 size_pixels) {
        const auto& io = ImGui::GetIO();

#ifdef IMGUI_ENABLE_FREETYPE
        io.Fonts->FontLoader = ImGuiFreeType::GetFontLoader();
        io.Fonts->FontLoaderFlags = ImGuiFreeTypeBuilderFlags_LightHinting;
#endif

        ImFontConfig config;
        config.SizePixels = size_pixels;
        config.OversampleH = 3;
        config.OversampleV = 1;
        config.PixelSnapH = true;

        const auto font = io.Fonts->AddFontDefault(&config);

        if (!font) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to load default font");
            return nullptr;
        }

        STAR_LOG_INFO(LogCategory::Platform, "Default font loaded ({}px)", size_pixels);

        return font;
    }

    ImFont* ImGuiFontManager::add_font_from_file(const std::filesystem::path& font_path, const f32 size_pixels,
                                                 const FontGlyphRange& glyph_range) {
        FontConfiguration config(font_path, size_pixels);
        config.glyph_ranges = {glyph_range};
        return load_font(config);
    }

    ImFont* ImGuiFontManager::add_font_from_file(const std::filesystem::path& font_path, const f32 size_pixels,
                                                 const std::vector<FontGlyphRange>& glyph_ranges) {
        FontConfiguration config(font_path, size_pixels);
        config.glyph_ranges = glyph_ranges;
        return load_font(config);
    }

    bool ImGuiFontManager::build_fonts() {
        const auto& io = ImGui::GetIO();

#ifdef IMGUI_ENABLE_FREETYPE
        if (!io.Fonts->Build()) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to build font atlas with FreeType");
            return false;
        }
        STAR_LOG_INFO(LogCategory::Platform, "Font atlas built successfully with FreeType");
#else
        if (!io.Fonts->Build()) {
            STAR_LOG_ERROR(LogCategory::Platform, "Failed to build font atlas");
            return false;
        }
        STAR_LOG_INFO(LogCategory::Platform, "Font atlas built successfully");
#endif

        return true;
    }

} // namespace star::platform
