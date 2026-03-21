#pragma once

namespace star::platform {
    enum class NativeWindowHandleType {
        Default,
        Win32,
        X11,
        Wayland,
        Cocoa,
        UIKit,
    };

    struct PlatformData {
        void* native_window_handle = nullptr;
        void* native_display_type = nullptr;
        NativeWindowHandleType type = NativeWindowHandleType::Default;
    };
} // namespace star::platform
