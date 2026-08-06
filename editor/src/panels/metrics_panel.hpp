#pragma once

#include <array>
#include <vector>

#include "panel.hpp"
#include "star/graphics/device_context.hpp"

namespace star::graphics {
    class Device;
} // namespace star::graphics

namespace star::editor {
    class MetricsPanel final : public Panel {
      public:
        MetricsPanel();

        void set_device(graphics::Device* device) {
            m_device = device;
        }

        void on_update(f32 dt) override;
        void on_imgui_render() override;

      private:
        void update_frame_history(f32 dt);
        void render_timing_section() const;
        void render_gpu_section() const;
        void render_pass_table() const;
        void render_memory_section() const;
        void render_frame_graph() const;

        static constexpr size_t FRAME_HISTORY_SIZE = 120;

        graphics::Device* m_device = nullptr;
        graphics::FrameStats m_stats{};
        std::vector<graphics::ViewStats> m_views;

        std::array<float, FRAME_HISTORY_SIZE> m_frame_times{};
        size_t m_current_index = 0;
    };
} // namespace star::editor
