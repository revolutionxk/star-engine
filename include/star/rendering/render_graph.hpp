#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "star/core/types.hpp"

namespace star::graphics {
    class Device;
    class DeviceContext;
} // namespace star::graphics

namespace star::rendering {
    class IRenderPass;
    struct FrameContext;

    enum class PassScope : u8 {
        PerView,
        Global,
    };

    class RenderGraph {
      public:
        RenderGraph();
        ~RenderGraph();

        RenderGraph(const RenderGraph&) = delete;
        RenderGraph& operator=(const RenderGraph&) = delete;

        void add_pass(std::unique_ptr<IRenderPass> pass);
        void remove_pass(std::string_view name);
        void clear();

        [[nodiscard]] IRenderPass* find_pass(std::string_view name) const;

        template<typename T>
        [[nodiscard]] T* find_pass_of_type() const {
            for (const auto& pass : m_passes) {
                if (auto* casted = dynamic_cast<T*>(pass.get())) {
                    return casted;
                }
            }
            return nullptr;
        }

        void compile();

        void pre_render(const FrameContext& frame);
        void execute(const FrameContext& frame, graphics::DeviceContext& context);
        u32 execute_scope(PassScope scope, const FrameContext& frame, graphics::DeviceContext& context,
                          u32 start_view_id);

        void post_render(const FrameContext& frame) const;

        void reset(u32 width, u32 height, graphics::DeviceContext& context);

        [[nodiscard]] const std::vector<IRenderPass*>& ordered_view() const noexcept {
            return m_ordered;
        }

        [[nodiscard]] std::size_t size() const noexcept {
            return m_passes.size();
        }

        void invalidate() noexcept {
            m_dirty = true;
        }

      private:
        std::vector<std::unique_ptr<IRenderPass>> m_passes;
        std::vector<IRenderPass*> m_ordered;
        bool m_dirty{true};
    };
} // namespace star::rendering
