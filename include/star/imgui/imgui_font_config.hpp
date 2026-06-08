#pragma once

#include <filesystem>
#include <vector>

#include "star/core/common.hpp"
#include "star/core/types.hpp"

struct ImFont;
struct ImFontConfig;

namespace star::platform {
    struct FontGlyphRange {
        const char* name;
        const void* ranges;

        static FontGlyphRange default_range();
        static FontGlyphRange latin();
        static FontGlyphRange cyrillic();
        static FontGlyphRange japanese();
        static FontGlyphRange chinese_full();
        static FontGlyphRange chinese_simplified_common();
        static FontGlyphRange korean();
        static FontGlyphRange thai();
        static FontGlyphRange vietnamese();
    };

    struct FontConfiguration {
        std::filesystem::path font_path;
        f32 size_pixels = 16.0f;
        std::vector<FontGlyphRange> glyph_ranges;
        bool merge_mode = false;
        bool freetype_enabled = true;
        bool pixel_snap = true;

        u32 freetype_flags = 0;
        i32 oversample_h = 3;
        i32 oversample_v = 1;

        FontConfiguration() = default;

        explicit FontConfiguration(std::filesystem::path path, const f32 size = 16.0f)
            : font_path(std::move(path)), size_pixels(size) {
            glyph_ranges.push_back(FontGlyphRange::default_range());
        }
    };

    class STAR_EXPORT ImGuiFontManager {
      public:
        ImGuiFontManager() = default;
        ~ImGuiFontManager() = default;

        static ImFont* load_font(const FontConfiguration& config);

        static ImFont* load_default_font(f32 size_pixels = 16.0f);

        static ImFont* add_font_from_file(const std::filesystem::path& font_path, f32 size_pixels,
                                          const FontGlyphRange& glyph_range = FontGlyphRange::default_range());

        static ImFont* add_font_from_file(const std::filesystem::path& font_path, f32 size_pixels,
                                          const std::vector<FontGlyphRange>& glyph_ranges);

        static bool build_fonts();
    };

} // namespace star::platform
