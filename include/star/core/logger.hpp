#pragma once
#include <memory>
#include <string_view>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "types.hpp"

namespace star {
    enum class LogLevel {
        Trace = SPDLOG_LEVEL_TRACE,
        Debug = SPDLOG_LEVEL_DEBUG,
        Info = SPDLOG_LEVEL_INFO,
        Warn = SPDLOG_LEVEL_WARN,
        Error = SPDLOG_LEVEL_ERROR,
        Critical = SPDLOG_LEVEL_CRITICAL,
        Off = SPDLOG_LEVEL_OFF
    };

    enum class LogCategory {
        Core,
        Application,
        Platform,
        Rendering,
        Physics,
        Audio,
        Resources,
        Scripting,
        Network,
        Editor,
        Game,
        Scene,
        Graphics,
    };

    class STAR_EXPORT LogScope {
      public:
        explicit LogScope(std::string_view name);
        ~LogScope();

        LogScope(const LogScope&) = delete;
        LogScope& operator=(const LogScope&) = delete;

        static inline thread_local u32 s_depth = 0;

      private:
        std::string m_name;
    };

    class STAR_EXPORT Logger {
      public:
        static void initialize(LogLevel console_level = LogLevel::Info, LogLevel file_level = LogLevel::Trace,
                               std::string_view log_file_path = "logs/star_engine.log");

        static void shutdown();

        static std::shared_ptr<spdlog::logger>& get_core_logger();
        static std::shared_ptr<spdlog::logger>& get_client_logger();
        static std::shared_ptr<spdlog::logger>& get_category_logger(LogCategory category);

        static void set_core_level(LogLevel level);
        static void set_client_level(LogLevel level);
        static void set_category_level(LogCategory category, LogLevel level);

        static std::string get_indent();

      private:
        static std::shared_ptr<spdlog::logger> s_core_logger;
        static std::shared_ptr<spdlog::logger> s_client_logger;
        static std::unordered_map<LogCategory, std::shared_ptr<spdlog::logger>> s_category_loggers;
    };

    constexpr const char* log_category_to_string(LogCategory category) {
        switch (category) {
            case LogCategory::Core:
                return "CORE";
            case LogCategory::Application:
                return "APPLICATION";
            case LogCategory::Platform:
                return "PLATFORM";
            case LogCategory::Rendering:
                return "RENDER";
            case LogCategory::Physics:
                return "PHYSICS";
            case LogCategory::Audio:
                return "AUDIO";
            case LogCategory::Resources:
                return "RESOURCE";
            case LogCategory::Scripting:
                return "SCRIPT";
            case LogCategory::Network:
                return "NETWORK";
            case LogCategory::Editor:
                return "EDITOR";
            case LogCategory::Game:
                return "GAME";
            case LogCategory::Scene:
                return "SCENE";
            case LogCategory::Graphics:
                return "GRAPHICS";
            default:
                return "UNKNOWN";
        }
    }
} // namespace star

#define STAR_LOG_EMIT(logger_expr, level, ...)                                                                         \
    do {                                                                                                               \
        if (const auto& _star_logger = (logger_expr); _star_logger->should_log(level)) {                               \
            _star_logger->log(level, ::star::Logger::get_indent() + fmt::format(__VA_ARGS__));                          \
        }                                                                                                              \
    } while (0)

#define STAR_CORE_TRACE(...) STAR_LOG_EMIT(::star::Logger::get_core_logger(), spdlog::level::trace, __VA_ARGS__)
#define STAR_CORE_DEBUG(...) STAR_LOG_EMIT(::star::Logger::get_core_logger(), spdlog::level::debug, __VA_ARGS__)
#define STAR_CORE_INFO(...) STAR_LOG_EMIT(::star::Logger::get_core_logger(), spdlog::level::info, __VA_ARGS__)
#define STAR_CORE_WARN(...) STAR_LOG_EMIT(::star::Logger::get_core_logger(), spdlog::level::warn, __VA_ARGS__)
#define STAR_CORE_ERROR(...) STAR_LOG_EMIT(::star::Logger::get_core_logger(), spdlog::level::err, __VA_ARGS__)
#define STAR_CORE_CRITICAL(...) STAR_LOG_EMIT(::star::Logger::get_core_logger(), spdlog::level::critical, __VA_ARGS__)

#define STAR_TRACE(...) STAR_LOG_EMIT(::star::Logger::get_client_logger(), spdlog::level::trace, __VA_ARGS__)
#define STAR_DEBUG(...) STAR_LOG_EMIT(::star::Logger::get_client_logger(), spdlog::level::debug, __VA_ARGS__)
#define STAR_INFO(...) STAR_LOG_EMIT(::star::Logger::get_client_logger(), spdlog::level::info, __VA_ARGS__)
#define STAR_WARN(...) STAR_LOG_EMIT(::star::Logger::get_client_logger(), spdlog::level::warn, __VA_ARGS__)
#define STAR_ERROR(...) STAR_LOG_EMIT(::star::Logger::get_client_logger(), spdlog::level::err, __VA_ARGS__)
#define STAR_CRITICAL(...) STAR_LOG_EMIT(::star::Logger::get_client_logger(), spdlog::level::critical, __VA_ARGS__)

#define STAR_LOG_TRACE(category, ...)                                                                                  \
    STAR_LOG_EMIT(::star::Logger::get_category_logger(category), spdlog::level::trace, __VA_ARGS__)
#define STAR_LOG_DEBUG(category, ...)                                                                                  \
    STAR_LOG_EMIT(::star::Logger::get_category_logger(category), spdlog::level::debug, __VA_ARGS__)
#define STAR_LOG_INFO(category, ...)                                                                                   \
    STAR_LOG_EMIT(::star::Logger::get_category_logger(category), spdlog::level::info, __VA_ARGS__)
#define STAR_LOG_WARN(category, ...)                                                                                   \
    STAR_LOG_EMIT(::star::Logger::get_category_logger(category), spdlog::level::warn, __VA_ARGS__)
#define STAR_LOG_ERROR(category, ...)                                                                                  \
    STAR_LOG_EMIT(::star::Logger::get_category_logger(category), spdlog::level::err, __VA_ARGS__)
#define STAR_LOG_CRITICAL(category, ...)                                                                               \
    STAR_LOG_EMIT(::star::Logger::get_category_logger(category), spdlog::level::critical, __VA_ARGS__)

#define STAR_LOG_SCOPE(name) ::star::LogScope STAR_CONCAT(__log_scope_, __LINE__)(name)
#define STAR_CONCAT(a, b) STAR_CONCAT_IMPL(a, b)
#define STAR_CONCAT_IMPL(a, b) a##b
