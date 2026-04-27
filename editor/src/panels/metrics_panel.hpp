#pragma once

#include <array>

#include "panel.hpp"

namespace star::editor {
    class MetricsPanel final : public Panel {
      public:
        MetricsPanel();

        void on_update(f32 dt) override;
        void on_imgui_render() override;

      private:
        void update_frame_history(f32 dt);
        static void render_performance_section();
        static void render_performance_indicator(float fps);
        void render_frame_graph() const;

        static constexpr size_t FRAME_HISTORY_SIZE = 120;
        std::array<float, FRAME_HISTORY_SIZE> m_frame_times{};
        size_t m_current_index = 0;
    };
} // namespace star::editor
