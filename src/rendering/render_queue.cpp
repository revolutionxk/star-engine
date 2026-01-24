#include "star/rendering/render_queue.hpp"

#include <algorithm>

#include "star/core/common.hpp"

namespace star::rendering {
    RenderQueue::RenderQueue() {
        m_opaque_commands.reserve(1024);
        m_transparent_commands.reserve(256);
    }

    RenderQueue::~RenderQueue() {
        clear();
    }

    void RenderQueue::submit(const DrawCall& draw_call) {
        if (draw_call.is_transparent) {
            m_transparent_commands.push_back(draw_call);
        } else {
            m_opaque_commands.push_back(draw_call);
        }
    }

    void RenderQueue::sort() {
        std::ranges::sort(m_opaque_commands, [](const DrawCall& a, const DrawCall& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }

            if (a.material.id != b.material.id) {
                return a.material.id < b.material.id;
            }

            return a.distance_to_camera < b.distance_to_camera;
        });

        std::ranges::sort(m_transparent_commands, [](const DrawCall& a, const DrawCall& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }

            return a.distance_to_camera > b.distance_to_camera;
        });

        STAR_LOG_TRACE(LogCategory::Rendering, "RenderQueue sorted: {} opaque, {} transparent",
                       m_opaque_commands.size(), m_transparent_commands.size());
    }

    void RenderQueue::clear() {
        m_opaque_commands.clear();
        m_transparent_commands.clear();
    }

    u64 RenderQueue::calculate_sort_key(const DrawCall& draw_call) {
        u64 key = 0;

        key |= static_cast<u64>(draw_call.layer) << 56;

        const u32 material_hash = draw_call.material.id & 0xFFFFFF;
        key |= static_cast<u64>(material_hash) << 32;

        u32 distance_bits;
        std::memcpy(&distance_bits, &draw_call.distance_to_camera, sizeof(u32));

        if (draw_call.is_transparent) {
            distance_bits = ~distance_bits;
        }

        key |= distance_bits;

        return key;
    }
} // namespace star::rendering
