#pragma once

#include "panel.hpp"
#include <imgui.h>
#include <array>

namespace star::editor {

    class MetricsPanel final : public Panel {
    public:
        MetricsPanel() : Panel("Metrics") {
            m_frame_times.fill(0.0f);
        }

        void on_update(const f32 dt) override {
            update_frame_history(dt);
        }

        void on_imgui_render() override {
            if (!m_is_open) return;

            ImGui::Begin(m_name.c_str(), &m_is_open);

            render_performance_section();
            ImGui::Spacing();
            render_frame_graph();

            ImGui::End();
        }

    private:
        void update_frame_history(f32 dt) {
            m_frame_times[m_current_index] = dt * 1000.0f; // Convert to ms
            m_current_index = (m_current_index + 1) % m_frame_times.size();
        }

        void render_performance_section() const {
            ImGui::SeparatorText("Performance");

            const ImGuiIO& io = ImGui::GetIO();
            const float fps = io.Framerate;
            const float frame_time = 1000.0f / fps;

            ImGui::Text("FPS: %.1f", fps);
            ImGui::Text("Frame Time: %.3f ms", frame_time);
            
            render_performance_indicator(fps);
        }

        static void render_performance_indicator(float fps) {
            ImVec4 color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
            
            if (fps < 30.0f) {
                color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
            } else if (fps < 60.0f) {
                color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
            }
            
            ImGui::TextColored(color, "Status: %s", 
                fps >= 60.0f ? "Excellent" : fps >= 30.0f ? "Good" : "Poor");
        }

        void render_frame_graph() const {
            ImGui::SeparatorText("Frame History");
            
            constexpr float max_frame_time = 33.33f; // 30 FPS baseline
            
            ImGui::PlotLines(
                "##FrameTime",
                m_frame_times.data(),
                static_cast<int>(m_frame_times.size()),
                m_current_index,
                "ms",
                0.0f,
                max_frame_time,
                ImVec2(0, 80)
            );
        }

        static constexpr size_t FRAME_HISTORY_SIZE = 120;
        std::array<float, FRAME_HISTORY_SIZE> m_frame_times;
        size_t m_current_index = 0;
    };

} // namespace star::editor
