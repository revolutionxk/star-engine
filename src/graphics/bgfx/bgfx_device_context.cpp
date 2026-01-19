#include "bgfx_device_context.hpp"

namespace star::graphics {
    BGFXDeviceContext::BGFXDeviceContext(void* native_window_handle) : DeviceContext(native_window_handle) {}

    BGFXDeviceContext::~BGFXDeviceContext() {}

    void BGFXDeviceContext::begin_frame() {}

    void BGFXDeviceContext::present() {}

    void BGFXDeviceContext::end_frame() {}

    void BGFXDeviceContext::resize(u32 width, u32 height) {}
} // namespace star::graphics
