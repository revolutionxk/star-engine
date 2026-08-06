#include "star/rendering/render_graph.hpp"

#include <algorithm>
#include <unordered_map>
#include <unordered_set>

#include "star/core/common.hpp"
#include "star/core/logger.hpp"
#include "star/graphics/device_context.hpp"
#include "star/rendering/renderer.hpp"

namespace star::rendering {
    RenderGraph::RenderGraph() = default;
    RenderGraph::~RenderGraph() = default;

    void RenderGraph::add_pass(std::unique_ptr<IRenderPass> pass) {
        if (!pass) {
            STAR_LOG_WARN(LogCategory::Rendering, "RenderGraph: rejected null pass");
            return;
        }

        STAR_LOG_INFO(LogCategory::Rendering, "RenderGraph: adding pass '{}' (priority {})", pass->get_name(),
                      pass->get_priority());

        m_passes.push_back(std::move(pass));
        m_dirty = true;
    }

    void RenderGraph::remove_pass(std::string_view name) {
        const auto erased = std::erase_if(
            m_passes, [name](const std::unique_ptr<IRenderPass>& pass) { return pass->get_name() == name; });

        if (erased > 0) {
            STAR_LOG_INFO(LogCategory::Rendering, "RenderGraph: removed pass '{}'", name);
            m_dirty = true;
        } else {
            STAR_LOG_WARN(LogCategory::Rendering, "RenderGraph: pass '{}' not found", name);
        }
    }

    void RenderGraph::clear() {
        m_passes.clear();
        m_ordered.clear();
        m_dirty = false;
    }

    IRenderPass* RenderGraph::find_pass(std::string_view name) const {
        for (const auto& pass : m_passes) {
            if (pass->get_name() == name) {
                return pass.get();
            }
        }
        return nullptr;
    }

    void RenderGraph::compile() {
        if (!m_dirty) {
            return;
        }

        m_ordered.clear();
        m_ordered.reserve(m_passes.size());

        std::unordered_map<std::string, std::size_t> name_to_index;
        name_to_index.reserve(m_passes.size());
        for (std::size_t i = 0; i < m_passes.size(); ++i) {
            name_to_index.emplace(m_passes[i]->get_name(), i);
        }

        std::vector<std::vector<std::size_t>> adj(m_passes.size());
        std::vector<u32> in_degree(m_passes.size(), 0);

        for (std::size_t i = 0; i < m_passes.size(); ++i) {
            for (const auto dep_name : m_passes[i]->dependencies()) {
                const auto it = name_to_index.find(std::string(dep_name));
                if (it == name_to_index.end()) {
                    STAR_LOG_WARN(LogCategory::Rendering,
                                  "RenderGraph: pass '{}' depends on missing pass '{}', edge ignored",
                                  m_passes[i]->get_name(), dep_name);
                    continue;
                }
                adj[it->second].push_back(i);
                ++in_degree[i];
            }
        }

        const auto priority_cmp = [this](const std::size_t a, const std::size_t b) {
            const u8 pa = m_passes[a]->get_priority();
            const u8 pb = m_passes[b]->get_priority();
            if (pa != pb) {
                return pa > pb;
            }
            return a > b;
        };

        std::vector<std::size_t> ready;
        ready.reserve(m_passes.size());
        for (std::size_t i = 0; i < m_passes.size(); ++i) {
            if (in_degree[i] == 0) {
                ready.push_back(i);
            }
        }
        std::ranges::make_heap(ready, priority_cmp);

        while (!ready.empty()) {
            std::ranges::pop_heap(ready, priority_cmp);
            const std::size_t idx = ready.back();
            ready.pop_back();

            m_ordered.push_back(m_passes[idx].get());

            for (const std::size_t next : adj[idx]) {
                if (--in_degree[next] == 0) {
                    ready.push_back(next);
                    std::ranges::push_heap(ready, priority_cmp);
                }
            }
        }

        if (m_ordered.size() != m_passes.size()) {
            STAR_LOG_ERROR(LogCategory::Rendering,
                           "RenderGraph: dependency cycle detected ({} of {} passes scheduled), falling back to "
                           "priority-only order",
                           m_ordered.size(), m_passes.size());

            m_ordered.clear();
            for (const auto& pass : m_passes) {
                m_ordered.push_back(pass.get());
            }
            std::ranges::sort(m_ordered, [](const IRenderPass* a, const IRenderPass* b) {
                return a->get_priority() < b->get_priority();
            });
        }

        STAR_LOG_DEBUG(LogCategory::Rendering, "RenderGraph: compiled {} passes", m_ordered.size());
        for (const auto* pass : m_ordered) {
            STAR_LOG_DEBUG(LogCategory::Rendering, "  - {} (priority {})", pass->get_name(), pass->get_priority());
        }

        m_dirty = false;
    }

    void RenderGraph::pre_render(const FrameContext& frame) {
        compile();
        for (auto* pass : m_ordered) {
            if (pass->is_enabled()) {
                pass->pre_render(frame);
            }
        }
    }

    u32 RenderGraph::execute_scope(const PassScope scope, const FrameContext& frame,
                                   graphics::DeviceContext& context, u32 start_view_id) {
        compile();
        for (auto* pass : m_ordered) {
            if (!pass->is_enabled() || pass->scope() != scope)
                continue;
            const RenderContext ctx{frame, context, start_view_id++};
            context.set_view_name(ctx.view_id, pass->get_name());
            pass->render(ctx);
        }
        return start_view_id;
    }

    void RenderGraph::post_render(const FrameContext& frame) const {
        for (auto* pass : m_ordered) {
            if (pass->is_enabled()) {
                pass->post_render(frame);
            }
        }
    }

    void RenderGraph::on_resize(const u32 width, const u32 height) {
        compile();
        for (auto* pass : m_ordered) {
            pass->on_resize(width, height);
        }
    }

} // namespace star::rendering
