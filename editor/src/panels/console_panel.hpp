#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "../core/editor_log_sink.hpp"
#include "imgui.h"
#include "panel.hpp"

namespace star::editor {
    struct LogEntry {
        std::string message;
        LogLevel level;
    };

    class ConsolePanel final : public Panel {
      public:
        ConsolePanel() : Panel("Console") {}

        void on_attach() override;
        void on_detach() override;
        void on_update(f32 dt) override;
        void on_imgui_render() override;

        void add_pending(std::string message, LogLevel level);

      private:
        void render_toolbar();
        void render_log_content();
        [[nodiscard]] bool should_display_log(LogLevel level) const noexcept;
        [[nodiscard]] static constexpr ImVec4 get_log_color(LogLevel level) noexcept;
        [[nodiscard]] static constexpr const char* get_log_prefix(LogLevel level) noexcept;
        void clear_logs();

        std::shared_ptr<spdlog::sinks::sink> m_sink;
        std::mutex m_pending_mutex;
        std::vector<LogEntry> m_pending;
        std::vector<LogEntry> m_logs;
        bool m_should_scroll_to_bottom = false;
        bool m_show_info = true;
        bool m_show_warnings = true;
        bool m_show_errors = true;
    };
} // namespace star::editor
