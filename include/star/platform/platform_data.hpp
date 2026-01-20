#pragma once

namespace star::platform {
    struct PlatformData {
        void* native_window_handle = nullptr;
        void* native_display_type = nullptr;
    };
} // namespace star::platform
