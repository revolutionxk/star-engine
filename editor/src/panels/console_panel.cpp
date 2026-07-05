#include "console_panel.hpp"

#include <imgui.h>
#include <spdlog/spdlog.h>

namespace star::editor {
    void ConsolePanel::on_attach() {
        const auto sink = std::make_shared<EditorLogSink_mt>(
            [this](std::string msg, const LogLevel level) { add_pending(std::move(msg), level); });
        m_sink = sink;
        spdlog::default_logger()->sinks().push_back(sink);
    }

    void ConsolePanel::on_detach() {
        if (!m_sink)
            return;

        if (const auto logger = spdlog::default_logger()) {
            auto& sinks = logger->sinks();
            sinks.erase(std::ranges::remove(sinks, m_sink).begin(), sinks.end());
        }
        m_sink.reset();
    }

    void ConsolePanel::on_update(f32 /*dt*/) {
        std::vector<LogEntry> batch;
        {
            std::scoped_lock lock(m_pending_mutex);
            batch.swap(m_pending);
        }
        for (auto& entry : batch) {
            m_logs.push_back(std::move(entry));
            m_should_scroll_to_bottom = true;
        }
    }

    void ConsolePanel::on_imgui_render() {
        if (!m_is_open)
            return;

        ImGui::Begin(m_name.c_str(), &m_is_open);

        render_toolbar();
        ImGui::Separator();
        render_log_content();

        ImGui::End();
    }

    void ConsolePanel::add_pending(std::string message, const LogLevel level) {
        std::scoped_lock lock(m_pending_mutex);
        m_pending.emplace_back(LogEntry{std::move(message), level});
    }

    void ConsolePanel::render_toolbar() {
        if (ImGui::Button("Clear"))
            clear_logs();

        ImGui::SameLine();
        ImGui::Checkbox("Info", &m_show_info);
        ImGui::SameLine();
        ImGui::Checkbox("Warning", &m_show_warnings);
        ImGui::SameLine();
        ImGui::Checkbox("Error", &m_show_errors);
    }

    void ConsolePanel::render_log_content() {
        ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& [message, level] : m_logs) {
            if (!should_display_log(level))
                continue;

            ImGui::TextColored(get_log_color(level), "[%s] %s", get_log_prefix(level), message.c_str());
        }

        if (m_should_scroll_to_bottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
            ImGui::SetScrollHereY(1.0f);
            m_should_scroll_to_bottom = false;
        }

        ImGui::EndChild();
    }

    bool ConsolePanel::should_display_log(const LogLevel level) const noexcept {
        switch (level) {
            case LogLevel::Info:
                return m_show_info;
            case LogLevel::Warning:
                return m_show_warnings;
            case LogLevel::Error:
                return m_show_errors;
            default:
                return true;
        }
    }

    constexpr ImVec4 ConsolePanel::get_log_color(const LogLevel level) noexcept {
        switch (level) {
            case LogLevel::Info:
                return {1.0f, 1.0f, 1.0f, 1.0f};
            case LogLevel::Warning:
                return {1.0f, 0.8f, 0.0f, 1.0f};
            case LogLevel::Error:
                return {1.0f, 0.2f, 0.2f, 1.0f};
            default:
                return {1.0f, 1.0f, 1.0f, 1.0f};
        }
    }

    constexpr const char* ConsolePanel::get_log_prefix(const LogLevel level) noexcept {
        switch (level) {
            case LogLevel::Info:
                return "Info";
            case LogLevel::Warning:
                return "Warn";
            case LogLevel::Error:
                return "Error";
            default:
                return "Log";
        }
    }

    void ConsolePanel::clear_logs() {
        m_logs.clear();
    }
} // namespace star::editor
