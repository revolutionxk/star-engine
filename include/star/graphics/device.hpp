#pragma once
#include "star/platform/platform_data.hpp"

namespace star::graphics {
    class DeviceContext;
    enum class GraphicsAPI {
        Auto,
        BGFX,
    };

    struct GraphicsDeviceConfig {
        GraphicsAPI api = GraphicsAPI::Auto;
        platform::PlatformData platform{};
        Vector2 window_size{};
    };

    class Device {
      public:
        virtual ~Device() = default;

        virtual void initialize(GraphicsDeviceConfig& device) = 0;
        virtual void shutdown() = 0;

        virtual std::unique_ptr<DeviceContext> create_context() = 0;

        static std::unique_ptr<Device> create(GraphicsAPI api = GraphicsAPI::Auto);

      private:
        GraphicsDeviceConfig config;
    };
} // namespace star::graphics
