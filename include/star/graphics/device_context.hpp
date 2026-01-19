#pragma once

namespace star::platform {
    class Window;
}

namespace star::graphics {
    class DeviceContext {
      public:
        explicit DeviceContext(void* native_window_handle);
        virtual ~DeviceContext() = default;

        virtual void begin_frame() = 0;
        virtual void present() = 0;
        virtual void end_frame() = 0;

        virtual void resize(u32 width, u32 height) = 0;

        void* native_window_handle() const {
            return m_native_window_handle;
        }

      protected:
        void* m_native_window_handle = nullptr;
    };
} // namespace star::graphics
