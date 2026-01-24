#pragma once

#include <string>
#include <vector>

#include <imgui.h>

#include "panel.hpp"

namespace star::editor {

    enum class LogLevel {
        Info,
        Warning,
        Error
    };

    struct LogEntry {
        std::string message;
        LogLevel level;
    };

    class ConsolePanel final : public Panel {
      public:
        ConsolePanel() : Panel("Console") {
            add_log("Editor initialized", LogLevel::Info);
            add_log("Teste", LogLevel::Warning);
            add_log("Teste", LogLevel::Error);
        }

        void add_log(std::string message, const LogLevel level = LogLevel::Info) {
            m_logs.emplace_back(LogEntry{std::move(message), level});
            m_should_scroll_to_bottom = true;
        }

        void on_imgui_render() override {
            if (!m_is_open)
                return;

            ImGui::Begin(m_name.c_str(), &m_is_open);

            render_toolbar();
            ImGui::Separator();
            render_log_content();

            ImGui::End();
        }

      private:
        void render_toolbar() {
            if (ImGui::Button("Clear")) {
                clear_logs();
            }

            ImGui::SameLine();
            ImGui::Checkbox("Info", &m_show_info);
            ImGui::SameLine();
            ImGui::Checkbox("Warning", &m_show_warnings);
            ImGui::SameLine();
            ImGui::Checkbox("Error", &m_show_errors);
        }

        void render_log_content() {
            ImGui::BeginChild("LogScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

            for (const auto& [message, level] : m_logs) {
                if (!should_display_log(level))
                    continue;

                const ImVec4 color = get_log_color(level);
                const char* prefix = get_log_prefix(level);

                ImGui::TextColored(color, "[%s] %s", prefix, message.c_str());
            }

            if (m_should_scroll_to_bottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
                ImGui::SetScrollHereY(1.0f);
                m_should_scroll_to_bottom = false;
            }

            ImGui::EndChild();
        }

        [[nodiscard]] bool should_display_log(const LogLevel level) const noexcept {
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

        [[nodiscard]] static constexpr ImVec4 get_log_color(const LogLevel level) noexcept {
            switch (level) {
                case LogLevel::Info:
                    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                case LogLevel::Warning:
                    return ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
                case LogLevel::Error:
                    return ImVec4(1.0f, 0.2f, 0.2f, 1.0f);
                default:
                    return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        [[nodiscard]] static constexpr const char* get_log_prefix(const LogLevel level) noexcept {
            switch (level) {
                case LogLevel::Info:
                    return "Info";
                case LogLevel::Warning:
                    return "Warning";
                case LogLevel::Error:
                    return "Error";
                default:
                    return "Log";
            }
        }

        void clear_logs() {
            m_logs.clear();
        }

        std::vector<LogEntry> m_logs;
        bool m_should_scroll_to_bottom = false;
        bool m_show_info = true;
        bool m_show_warnings = true;
        bool m_show_errors = true;
    };

} // namespace star::editor
