#pragma once

namespace star::platform {
    enum class WindowMode {
        Windowed,
        Fullscreen,
        Borderless
    };

    struct STAR_EXPORT VideoMode {
        Vector2 size{1280, 720};
        int display_index{0};
        WindowMode mode{WindowMode::Windowed};
        bool resizable{true};
        bool vsync{true};

        bool operator==(const VideoMode& other) const {
            return size == other.size && display_index == other.display_index && mode == other.mode &&
                   vsync == other.vsync;
        }

        bool operator!=(const VideoMode& other) const {
            return !(*this == other);
        }
    };
} // namespace star::platform
