#include "star/rendering/view_allocator.hpp"

#include "star/core/common.hpp"
#include "star/graphics/device_context.hpp"

namespace star::rendering {
    u16 ViewAllocator::acquire(const std::string_view debug_name) {
        return acquire_range(debug_name, 1);
    }

    u16 ViewAllocator::acquire_range(const std::string_view debug_name, const u16 count) {
        if (count == 0) {
            return m_next;
        }

        if (m_next + count > MAX_VIEWS) {
            if (!m_overflowed) {
                m_overflowed = true;
                STAR_LOG_ERROR(LogCategory::Rendering,
                               "ViewAllocator: out of views requesting {} for '{}' ({} of {} used)", count, debug_name,
                               m_next, MAX_VIEWS);
            }
            return MAX_VIEWS - 1;
        }

        const u16 base = m_next;
        m_next += count;

        m_context->set_view_name(base, debug_name);

        return base;
    }
} // namespace star::rendering
