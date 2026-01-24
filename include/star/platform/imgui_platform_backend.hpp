#pragma once

namespace star::platform {
    class IImGuiPlatformBackend {
      public:
        virtual ~IImGuiPlatformBackend() = default;

        virtual bool initialize(void* window_handle) = 0;
        virtual void shutdown() = 0;
        virtual void new_frame() = 0;

        [[nodiscard]] virtual bool is_initialized() const = 0;
    };

} // namespace star::platform
