#include "metrics_panel.hpp"

#include <imgui.h>

namespace star::editor {
    MetricsPanel::MetricsPanel() : Panel("Metrics") {
        m_frame_times.fill(0.0f);
    }

    void MetricsPanel::on_update(const f32 dt) {
        update_frame_history(dt);
    }

    void MetricsPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        render_performance_section();
        ImGui::Spacing();
        render_frame_graph();

        ImGui::End();
    }

    void MetricsPanel::update_frame_history(const f32 dt) {
        m_frame_times[m_current_index] = dt * 1000.0f;
        m_current_index = (m_current_index + 1) % m_frame_times.size();
    }

    void MetricsPanel::render_performance_section() {
        ImGui::SeparatorText("Performance");

        const ImGuiIO& io = ImGui::GetIO();
        const float fps = io.Framerate;
        const float frame_time = 1000.0f / fps;

        ImGui::Text("FPS: %.1f", fps);
        ImGui::Text("Frame Time: %.3f ms", frame_time);

        render_performance_indicator(fps);
    }

    void MetricsPanel::render_performance_indicator(const float fps) {
        ImVec4 color = {0.0f, 1.0f, 0.0f, 1.0f};
        if (fps < 30.0f)
            color = {1.0f, 0.0f, 0.0f, 1.0f};
        else if (fps < 60.0f)
            color = {1.0f, 1.0f, 0.0f, 1.0f};

        ImGui::TextColored(color, "Status: %s", fps >= 60.0f ? "Excellent" : fps >= 30.0f ? "Good" : "Poor");
    }

    void MetricsPanel::render_frame_graph() const {
        ImGui::SeparatorText("Frame History");

        constexpr float max_frame_time = 33.33f;
        ImGui::PlotLines("##FrameTime", m_frame_times.data(), static_cast<int>(m_frame_times.size()),
                         static_cast<int>(m_current_index), "ms", 0.0f, max_frame_time, ImVec2(0, 80));
    }
} // namespace star::editor
