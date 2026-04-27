#pragma once

#include <functional>
#include <mutex>

#include <spdlog/details/log_msg.h>
#include <spdlog/sinks/base_sink.h>

namespace star::editor {
    enum class LogLevel : u8 {
        Info,
        Warning,
        Error
    };

    template<typename Mutex>
    class EditorLogSink final : public spdlog::sinks::base_sink<Mutex> {
      public:
        using Callback = std::function<void(std::string, LogLevel)>;

        explicit EditorLogSink(Callback callback) : m_callback(std::move(callback)) {}

      protected:
        void sink_it_(const spdlog::details::log_msg& msg) override {
            spdlog::memory_buf_t buf;
            this->formatter_->format(msg, buf);
            m_callback(fmt::to_string(buf), to_log_level(msg.level));
        }

        void flush_() override {}

      private:
        static LogLevel to_log_level(const spdlog::level::level_enum level) noexcept {
            if (level >= spdlog::level::err)
                return LogLevel::Error;
            if (level >= spdlog::level::warn)
                return LogLevel::Warning;
            return LogLevel::Info;
        }

        Callback m_callback;
    };

    using EditorLogSink_mt = EditorLogSink<std::mutex>;
} // namespace star::editor
