#include "star/graphics/device_context.hpp"

namespace star::graphics {
    DeviceContext::DeviceContext(Device* device, void* native_window_handle)
        : m_device(device), m_native_window_handle(native_window_handle) {}
} // namespace star::graphics
