#pragma once
#include "star/graphics/device_context.hpp"

namespace star::graphics {
    class BGFXDeviceContext : public DeviceContext {
      public:
        explicit BGFXDeviceContext(void* native_window_handle);
        ~BGFXDeviceContext() override;

        void begin_frame() override;
        void present() override;
        void end_frame() override;

        void resize(u32 width, u32 height) override;
    };
} // namespace star::graphics
