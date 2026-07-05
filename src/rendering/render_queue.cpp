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

            return a.distance_sq < b.distance_sq;
        });

        std::ranges::sort(m_transparent_commands, [](const DrawCall& a, const DrawCall& b) {
            if (a.layer != b.layer) {
                return a.layer < b.layer;
            }

            return a.distance_sq > b.distance_sq;
        });

        STAR_LOG_TRACE(LogCategory::Rendering, "RenderQueue sorted: {} opaque, {} transparent",
                       m_opaque_commands.size(), m_transparent_commands.size());
    }

    void RenderQueue::clear() {
        m_opaque_commands.clear();
        m_transparent_commands.clear();
    }
} // namespace star::rendering
