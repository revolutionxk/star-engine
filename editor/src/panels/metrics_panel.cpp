#include "metrics_panel.hpp"

#include <imgui.h>

#include "star/graphics/device.hpp"

namespace star::editor {
    constexpr f64 BYTES_PER_MIB = 1024.0 * 1024.0;

    void metric_row(const char* label, const char* fmt, ...) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::TextUnformatted(label);
        ImGui::TableNextColumn();

        va_list args;
        va_start(args, fmt);
        ImGui::TextV(fmt, args);
        va_end(args);
    }

    ImVec4 budget_color(const f64 ms) {
        if (ms > 16.6)
            return {0.90f, 0.45f, 0.42f, 1.0f};
        if (ms > 11.0)
            return {0.95f, 0.75f, 0.35f, 1.0f};
        return {0.42f, 0.82f, 0.50f, 1.0f};
    }

    MetricsPanel::MetricsPanel() : Panel("Metrics") {
        m_frame_times.fill(0.0f);
    }

    void MetricsPanel::on_update(const f32 dt) {
        update_frame_history(dt);

        if (!m_device) {
            return;
        }
        if (const auto* context = m_device->context()) {
            m_stats = context->frame_stats();
            context->collect_view_stats(m_views);
        }
    }

    void MetricsPanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        render_timing_section();
        ImGui::Spacing();
        render_frame_graph();
        ImGui::Spacing();
        render_gpu_section();
        ImGui::Spacing();
        render_pass_table();
        ImGui::Spacing();
        render_memory_section();

        ImGui::End();
    }

    void MetricsPanel::update_frame_history(const f32 dt) {
        m_frame_times[m_current_index] = dt * 1000.0f;
        m_current_index = (m_current_index + 1) % m_frame_times.size();
    }

    void MetricsPanel::render_timing_section() const {
        ImGui::SeparatorText("Timing");

        const f64 cpu = m_stats.cpu_frame_ms;
        const f64 gpu = m_stats.gpu_frame_ms;
        const f64 frame = cpu > 0.0 ? cpu : 1000.0 / static_cast<f64>(ImGui::GetIO().Framerate);

        ImGui::TextColored(budget_color(frame), "%.2f ms", frame);
        ImGui::SameLine();
        ImGui::TextDisabled("(%.0f fps)", frame > 0.0 ? 1000.0 / frame : 0.0);

        if (ImGui::BeginTable("##timing", 2, ImGuiTableFlags_SizingStretchProp)) {
            metric_row("CPU", "%.2f ms", cpu);
            metric_row("GPU", "%.2f ms", gpu);
            metric_row("Wait submit", "%.2f ms", m_stats.wait_submit_ms);
            metric_row("Wait render", "%.2f ms", m_stats.wait_render_ms);
            ImGui::EndTable();
        }
    }

    void MetricsPanel::render_frame_graph() const {
        ImGui::SeparatorText("Frame history");

        constexpr float max_frame_time = 33.33f;
        ImGui::PlotLines("##FrameTime", m_frame_times.data(), static_cast<int>(m_frame_times.size()),
                         static_cast<int>(m_current_index), "ms", 0.0f, max_frame_time, ImVec2(0, 80));
    }

    void MetricsPanel::render_gpu_section() const {
        ImGui::SeparatorText("Submission");

        if (ImGui::BeginTable("##submission", 2, ImGuiTableFlags_SizingStretchProp)) {
            metric_row("Draw calls", "%u", m_stats.draw_calls);
            metric_row("Triangles", "%llu", static_cast<unsigned long long>(m_stats.triangles));
            metric_row("Compute", "%u", m_stats.compute_calls);
            metric_row("Blits", "%u", m_stats.blit_calls);
            ImGui::EndTable();
        }
    }

    void MetricsPanel::render_pass_table() const {
        ImGui::SeparatorText("Per pass");

        if (m_views.empty()) {
            ImGui::TextDisabled("No view timings this frame.");
            return;
        }

        constexpr ImGuiTableFlags flags =
            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp;

        if (ImGui::BeginTable("##passes", 4, flags)) {
            ImGui::TableSetupColumn("View");
            ImGui::TableSetupColumn("Pass");
            ImGui::TableSetupColumn("CPU");
            ImGui::TableSetupColumn("GPU");
            ImGui::TableHeadersRow();

            for (const auto& view : m_views) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("%u", view.view_id);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(view.name[0] != '\0' ? view.name : "-");
                ImGui::TableNextColumn();
                ImGui::Text("%.3f", view.cpu_ms);
                ImGui::TableNextColumn();
                ImGui::TextColored(budget_color(view.gpu_ms), "%.3f", view.gpu_ms);
            }
            ImGui::EndTable();
        }
    }

    void MetricsPanel::render_memory_section() const {
        ImGui::SeparatorText("Resources");

        if (ImGui::BeginTable("##resources", 2, ImGuiTableFlags_SizingStretchProp)) {
            metric_row("Textures", "%u  (%.1f MiB)", m_stats.textures,
                       static_cast<f64>(m_stats.texture_memory) / BYTES_PER_MIB);
            metric_row("Render targets", "%u  (%.1f MiB)", m_stats.framebuffers,
                       static_cast<f64>(m_stats.render_target_memory) / BYTES_PER_MIB);
            metric_row("Programs", "%u", m_stats.programs);
            metric_row("Uniforms", "%u", m_stats.uniform_count);
            metric_row("Transient VB", "%.1f KiB", static_cast<f64>(m_stats.transient_vb_used) / 1024.0);
            metric_row("Transient IB", "%.1f KiB", static_cast<f64>(m_stats.transient_ib_used) / 1024.0);
            ImGui::EndTable();
        }
    }
} // namespace star::editor
