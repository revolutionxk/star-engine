#pragma once

namespace star::platform {
    class IImGuiRenderer {
      public:
        virtual ~IImGuiRenderer() = default;
        virtual bool initialize() = 0;
        virtual void shutdown() = 0;
        virtual void render(u32 view_id) = 0;
        virtual void reset(u32 width, u32 height) = 0;
        [[nodiscard]] virtual bool is_initialized() const = 0;
    };

} // namespace star::platform
