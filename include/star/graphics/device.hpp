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

    struct DeviceCaps {
        std::string renderer_name{};
        std::string vendor_name{};
        std::string version_string{};

        int max_texture_size = 0;
        int max_texture_units = 0;
        bool supports_compute_shaders = false;
    };

    class Device {
      public:
        virtual ~Device() = default;

        virtual void initialize(GraphicsDeviceConfig& device) = 0;
        virtual void shutdown() = 0;
        virtual std::string name() const = 0;
        virtual DeviceCaps caps() const = 0;

        virtual std::unique_ptr<DeviceContext> create_context() = 0;

        static std::unique_ptr<Device> create(GraphicsAPI api = GraphicsAPI::Auto);

      private:
        GraphicsDeviceConfig config;
    };
} // namespace star::graphics
