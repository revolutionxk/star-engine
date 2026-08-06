#include "star/rendering/view_resources.hpp"

#include <algorithm>

#include "star/core/common.hpp"
#include "star/graphics/device.hpp"

namespace star::rendering {
    void ViewResources::begin_frame() {
        m_published.clear();
    }

    void ViewResources::publish(const std::string_view id, const graphics::ResourceHandle<graphics::Texture> handle) {
        if (const auto it = m_published.find(id); it != m_published.end()) {
            it->second = handle;
            return;
        }
        m_published.emplace(std::string(id), handle);
    }

    graphics::ResourceHandle<graphics::Texture> ViewResources::texture(const std::string_view id) const {
        const auto it = m_published.find(id);
        return it != m_published.end() ? it->second : graphics::ResourceHandle<graphics::Texture>{};
    }

    RenderTarget* ViewResources::target(const std::string_view id, const u32 width, const u32 height,
                                        const graphics::TextureFormat format, const bool with_depth) {
        const u32 w = std::max(1u, width);
        const u32 h = std::max(1u, height);

        auto it = m_targets.find(id);
        if (it == m_targets.end()) {
            it = m_targets.emplace(std::string(id), Entry{}).first;
        }

        Entry& entry = it->second;
        if (entry.target && entry.target->is_valid() && entry.width == w && entry.height == h &&
            entry.format == format && entry.depth == with_depth) {
            return entry.target.get();
        }

        auto created = std::make_unique<RenderTarget>();
        if (!created->create(m_device, w, h, format, with_depth)) {
            STAR_LOG_ERROR(LogCategory::Rendering, "ViewResources: failed to create target '{}' ({}x{})", id, w, h);
            entry = Entry{};
            return nullptr;
        }

        entry.target = std::move(created);
        entry.width = w;
        entry.height = h;
        entry.format = format;
        entry.depth = with_depth;
        return entry.target.get();
    }

    RenderTarget* ViewResources::find(const std::string_view id) const {
        const auto it = m_targets.find(id);
        if (it == m_targets.end() || !it->second.target) {
            return nullptr;
        }
        return it->second.target.get();
    }

    u64& ViewResources::counter(const std::string_view id, const u64 initial) {
        const auto it = m_counters.find(id);
        if (it != m_counters.end()) {
            return it->second;
        }
        return m_counters.emplace(std::string(id), initial).first->second;
    }

    void ViewResources::clear() {
        for (auto& [name, entry] : m_targets) {
            if (entry.target) {
                entry.target->destroy();
            }
        }
        m_targets.clear();
        m_published.clear();
        m_counters.clear();
    }
} // namespace star::rendering
