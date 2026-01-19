#include "star/graphics/device.hpp"

#include "bgfx/bgfx_device.hpp"

namespace star::graphics {
    std::unique_ptr<Device> Device::create(const GraphicsAPI api) {
#if defined(STAR_GRAPHICS_API_BGFX)
        if (api == GraphicsAPI::Auto || api == GraphicsAPI::BGFX) {
            return std::make_unique<BGFXDevice>();
        }
#else
    #error "No graphics API defined! Define STAR_GRAPHICS_API_BGFX in CMake."
#endif
        return nullptr;
    }
} // namespace star::graphics
