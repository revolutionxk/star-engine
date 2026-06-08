#include "star/rendering/shader_hot_reloader.hpp"

#include <system_error>

#include "star/core/logger.hpp"
#include "star/resources/resource_manager.hpp"
#include "star/resources/shader/shader.hpp"

namespace star::rendering {
    ShaderHotReloader::ShaderHotReloader(resources::ResourceManager& rm) : m_resource_manager(rm) {}

    void ShaderHotReloader::watch(const graphics::ResourceHandle<graphics::Shader> handle) {
        if (!handle.is_valid())
            return;

        auto* shader = m_resource_manager.get_shader(handle);
        if (!shader) {
            STAR_LOG_WARN(LogCategory::Resources, "ShaderHotReloader::watch: invalid shader handle");
            return;
        }

        if (shader->disk_vertex_path.empty() || shader->disk_fragment_path.empty()) {
            STAR_LOG_DEBUG(LogCategory::Resources, "ShaderHotReloader::watch: skipping embedded shader '{}'",
                           shader->m_path);
            return;
        }

        for (const auto& e : m_entries) {
            if (e.handle.id == handle.id && e.handle.generation == handle.generation)
                return;
        }

        Entry entry;
        entry.handle = handle;
        entry.vertex_path = shader->disk_vertex_path;
        entry.fragment_path = shader->disk_fragment_path;

        std::error_code ec;
        entry.vertex_mtime = std::filesystem::last_write_time(entry.vertex_path, ec);
        if (ec)
            entry.vertex_mtime = {};
        entry.fragment_mtime = std::filesystem::last_write_time(entry.fragment_path, ec);
        if (ec)
            entry.fragment_mtime = {};

        m_entries.push_back(std::move(entry));
        STAR_LOG_INFO(LogCategory::Resources, "ShaderHotReloader: watching '{}'", shader->m_path);
    }

    void ShaderHotReloader::unwatch(const graphics::ResourceHandle<graphics::Shader> handle) {
        std::erase_if(m_entries, [&](const Entry& e) {
            return e.handle.id == handle.id && e.handle.generation == handle.generation;
        });
    }

    void ShaderHotReloader::clear() {
        m_entries.clear();
    }

    void ShaderHotReloader::tick(const f32 delta_time, const f32 poll_interval_seconds) {
        m_time_since_poll += delta_time;
        if (m_time_since_poll < poll_interval_seconds)
            return;
        m_time_since_poll = 0.0f;

        for (auto& [handle, vertex_path, fragment_path, vertex_mtime, fragment_mtime] : m_entries) {
            std::error_code ec_v;
            std::error_code ec_f;
            const auto vmtime = std::filesystem::last_write_time(vertex_path, ec_v);
            const auto fmtime = std::filesystem::last_write_time(fragment_path, ec_f);
            if (ec_v || ec_f)
                continue;

            const bool vertex_changed = vmtime != vertex_mtime;
            const bool fragment_changed = fmtime != fragment_mtime;
            if (!vertex_changed && !fragment_changed)
                continue;

            vertex_mtime = vmtime;
            fragment_mtime = fmtime;

            if (m_resource_manager.reload_shader_from_disk(handle)) {
                STAR_LOG_INFO(LogCategory::Resources, "ShaderHotReloader: reloaded shader (id {})", handle.id);
            }
        }
    }

} // namespace star::rendering
