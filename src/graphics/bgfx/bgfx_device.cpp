#include "bgfx_device.hpp"

#include <bgfx/bgfx.h>

#include "bgfx_device_context.hpp"
#include "star/core/common.hpp"
#include "star/platform/window.hpp"

namespace star::graphics {
    BGFXDevice::~BGFXDevice() {
        if (!m_initialized) {
            return;
        }

        shutdown();
    }

    std::unique_ptr<DeviceContext> BGFXDevice::create_context() {
        return std::make_unique<BGFXDeviceContext>(m_config.platform.native_window_handle);
    }

    bool BGFXDevice::initialize(GraphicsDeviceConfig& device) {
        bgfx::Init init;
        bgfx::PlatformData platform_data{};

        if (device.platform.native_window_handle == nullptr) {
            STAR_LOG_ERROR(LogCategory::Graphics, "No window available for BGFX initialization");
            return false;
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
            return false;
        }

        constexpr auto clear_hex = static_cast<uint32_t>(0x443355FF);

        bgfx::setPaletteColor(0, 0xFF000000);
        bgfx::setPaletteColor(1, clear_hex);
        bgfx::setPaletteColor(2, 0xFFFFFFFF);

        bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, clear_hex, 1.0f, 0);

        bgfx::setViewRect(0, 0, 0, init.resolution.width, init.resolution.height);

        m_initialized = true;

        const auto caps = bgfx::getCaps();
        STAR_LOG_INFO(LogCategory::Graphics, "Graphics API: {}", bgfx::getRendererName(caps->rendererType));

        return true;
    }

    void BGFXDevice::shutdown() {
        bgfx::shutdown();

        m_initialized = false;
    }

    DeviceCaps BGFXDevice::caps() const {
        const auto caps = bgfx::getCaps();

        DeviceCaps device_caps{
            .renderer_name = bgfx::getRendererName(caps->rendererType),
            .vendor_name = caps->vendorId == BGFX_PCI_ID_NONE ? "Unknown" : std::to_string(caps->vendorId),
            .max_texture_size = static_cast<int>(caps->limits.maxTextureSize),
            .max_texture_units = static_cast<int>(caps->limits.maxTextureSamplers),
            .supports_compute_shaders = (caps->supported & BGFX_CAPS_COMPUTE) != 0,
        };

        return device_caps;
    }
} // namespace star::graphics
