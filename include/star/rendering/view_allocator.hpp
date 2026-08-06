#pragma once

#include <string_view>

#include "star/core/types.hpp"

namespace star::graphics {
    class DeviceContext;
} // namespace star::graphics

namespace star::rendering {
    class ViewAllocator {
      public:
        static constexpr u16 MAX_VIEWS = 256;

        explicit ViewAllocator(graphics::DeviceContext& context) : m_context(&context) {}

        void begin_frame() noexcept {
            m_next = 0;
            m_overflowed = false;
        }

        [[nodiscard]] u16 acquire(std::string_view debug_name);
        [[nodiscard]] u16 acquire_range(std::string_view debug_name, u16 count);

        [[nodiscard]] u16 used() const noexcept {
            return m_next;
        }

        [[nodiscard]] bool overflowed() const noexcept {
            return m_overflowed;
        }

      private:
        graphics::DeviceContext* m_context;
        u16 m_next{0};
        bool m_overflowed{false};
    };
} // namespace star::rendering
