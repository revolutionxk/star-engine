#pragma once
#include "star/graphics/device.hpp"
#include "star/graphics/device_context.hpp"

namespace star::platform {
    class Window;
}

namespace star::graphics {
    class BGFXDevice final : public Device {
        using Super = DeviceContext;

      public:
        BGFXDevice() = default;
        ~BGFXDevice() override = default;

        std::unique_ptr<DeviceContext> create_context() override;

        std::string name() const override {
            return "BGFX";
        }

        DeviceCaps caps() const override;

        void initialize(GraphicsDeviceConfig& device) override;
        void shutdown() override;
    };
} // namespace star::graphics
