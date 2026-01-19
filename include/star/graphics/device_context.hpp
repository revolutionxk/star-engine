#pragma once

namespace star::platform {
    class Window;
}

namespace star::graphics {
    class DeviceContext {
      public:
        virtual ~DeviceContext() = default;

        virtual void initialize() = 0;
        virtual void begin_frame() = 0;
        virtual void present() = 0;
        virtual void end_frame() = 0;

        virtual void resize(u32 width, u32 height) = 0;
    };
} // namespace star::graphics
