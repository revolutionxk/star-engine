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

#define STAR_CORE_TRACE(...)                                                                                           \
    ::star::Logger::get_core_logger()->trace(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_CORE_DEBUG(...)                                                                                           \
    ::star::Logger::get_core_logger()->debug(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_CORE_INFO(...)                                                                                            \
    ::star::Logger::get_core_logger()->info(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_CORE_WARN(...)                                                                                            \
    ::star::Logger::get_core_logger()->warn(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_CORE_ERROR(...)                                                                                           \
    ::star::Logger::get_core_logger()->error(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_CORE_CRITICAL(...)                                                                                        \
    ::star::Logger::get_core_logger()->critical(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))

#define STAR_TRACE(...)                                                                                                \
    ::star::Logger::get_client_logger()->trace(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_DEBUG(...)                                                                                                \
    ::star::Logger::get_client_logger()->debug(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_INFO(...)                                                                                                 \
    ::star::Logger::get_client_logger()->info(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_WARN(...)                                                                                                 \
    ::star::Logger::get_client_logger()->warn(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_ERROR(...)                                                                                                \
    ::star::Logger::get_client_logger()->error(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_CRITICAL(...)                                                                                             \
    ::star::Logger::get_client_logger()->critical(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))

#define STAR_LOG_TRACE(category, ...)                                                                                  \
    ::star::Logger::get_category_logger(category)->trace(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_LOG_DEBUG(category, ...)                                                                                  \
    ::star::Logger::get_category_logger(category)->debug(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_LOG_INFO(category, ...)                                                                                   \
    ::star::Logger::get_category_logger(category)->info(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_LOG_WARN(category, ...)                                                                                   \
    ::star::Logger::get_category_logger(category)->warn(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_LOG_ERROR(category, ...)                                                                                  \
    ::star::Logger::get_category_logger(category)->error(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))
#define STAR_LOG_CRITICAL(category, ...)                                                                               \
    ::star::Logger::get_category_logger(category)->critical(::star::Logger::get_indent() + fmt::format(__VA_ARGS__))

#define STAR_LOG_SCOPE(name) ::star::LogScope STAR_CONCAT(__log_scope_, __LINE__)(name)
#define STAR_CONCAT(a, b) STAR_CONCAT_IMPL(a, b)
#define STAR_CONCAT_IMPL(a, b) a##b
