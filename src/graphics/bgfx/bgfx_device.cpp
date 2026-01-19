#include "bgfx_device.hpp"

#include <bgfx/bgfx.h>

#include "bgfx_device_context.hpp"
#include "star/core/common.hpp"
#include "star/platform/window.hpp"

namespace star::graphics {
    std::unique_ptr<DeviceContext> BGFXDevice::create_context() {
        return std::make_unique<BGFXDeviceContext>();
    }

    void BGFXDevice::initialize(GraphicsDeviceConfig& device) {
        bgfx::Init init;
        bgfx::PlatformData platform_data{};

        if (device.platform.native_window_handle == nullptr) {
            STAR_LOG_ERROR(LogCategory::Graphics, "No window available for BGFX initialization");
            return;
        }

        platform_data.nwh = device.platform.native_window_handle;
        platform_data.ndt = device.platform.native_display_type;

        init.platformData = platform_data;
        init.type = bgfx::RendererType::Count;

        const auto size = device.window_size;

        init.resolution.width = static_cast<u32>(size.x);
        init.resolution.height = static_cast<u32>(size.y);
        init.debug = false;
        init.resolution.reset = 0;

        if (!bgfx::init(init)) {
            STAR_LOG_ERROR(LogCategory::Graphics, "Failed to initialize bgfx");
            return;
        }

        constexpr auto clear_hex = static_cast<uint32_t>(0x443355FF);

        bgfx::setPaletteColor(0, 0xFF000000);
        bgfx::setPaletteColor(1, clear_hex);
        bgfx::setPaletteColor(2, 0xFFFFFFFF);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clear_hex, 1.0f, 0);

        bgfx::setViewRect(0, 0, 0, init.resolution.width, init.resolution.height);
    }

    void BGFXDevice::shutdown() {}
} // namespace star::graphics
