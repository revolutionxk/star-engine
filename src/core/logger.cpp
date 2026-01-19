#include "star/core/logger.hpp"

#include <filesystem>

#include <spdlog/async.h>
#include <spdlog/pattern_formatter.h>

namespace star {
    std::shared_ptr<spdlog::logger> Logger::s_core_logger;
    std::shared_ptr<spdlog::logger> Logger::s_client_logger;
    std::unordered_map<LogCategory, std::shared_ptr<spdlog::logger>> Logger::s_category_loggers;

    void Logger::initialize(LogLevel console_level, LogLevel file_level, std::string_view log_file_path) {
        if (const std::filesystem::path log_path(log_file_path); log_path.has_parent_path()) {
            std::filesystem::create_directories(log_path.parent_path());
        }

        std::vector<spdlog::sink_ptr> sinks;

        const auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(static_cast<spdlog::level::level_enum>(console_level));
        console_sink->set_pattern("%^[%T] [%n] [%l]%$ %v");
        sinks.push_back(console_sink);

        const auto file_sink =
            std::make_shared<spdlog::sinks::rotating_file_sink_mt>(std::string(log_file_path), 1024 * 1024 * 5, 3);
        file_sink->set_level(static_cast<spdlog::level::level_enum>(file_level));
        file_sink->set_pattern("[%Y-%m-%d %T.%e] [%n] [%l] [thread %t] %v");
        sinks.push_back(file_sink);

        s_core_logger = std::make_shared<spdlog::logger>("STAR", sinks.begin(), sinks.end());
        s_core_logger->set_level(spdlog::level::trace);
        s_core_logger->flush_on(spdlog::level::err);
        spdlog::register_logger(s_core_logger);

        s_client_logger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
        s_client_logger->set_level(spdlog::level::trace);
        s_client_logger->flush_on(spdlog::level::err);
        spdlog::register_logger(s_client_logger);

        for (int i = 0; i <= static_cast<int>(LogCategory::Graphics); ++i) {
            auto category = static_cast<LogCategory>(i);
            auto category_logger =
                std::make_shared<spdlog::logger>(log_category_to_string(category), sinks.begin(), sinks.end());
            category_logger->set_level(spdlog::level::trace);
            category_logger->flush_on(spdlog::level::err);
            spdlog::register_logger(category_logger);
            s_category_loggers[category] = category_logger;
        }

        STAR_CORE_INFO("Logger initialized");
        STAR_CORE_INFO("Console level: {}",
                       spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(console_level)));
        STAR_CORE_INFO("File level: {}",
                       spdlog::level::to_string_view(static_cast<spdlog::level::level_enum>(file_level)));
        STAR_CORE_INFO("Log file: {}", log_file_path);
    }

    void Logger::shutdown() {
        STAR_CORE_INFO("Logger shutting down");
        spdlog::shutdown();
    }

    std::shared_ptr<spdlog::logger>& Logger::get_core_logger() {
        return s_core_logger;
    }

    std::shared_ptr<spdlog::logger>& Logger::get_client_logger() {
        return s_client_logger;
    }

    std::shared_ptr<spdlog::logger>& Logger::get_category_logger(LogCategory category) {
        auto it = s_category_loggers.find(category);
        if (it == s_category_loggers.end()) {
            STAR_CORE_ERROR("Logger for category '{}' not found, using core logger", log_category_to_string(category));
            return s_core_logger;
        }
        return it->second;
    }

    void Logger::set_core_level(LogLevel level) {
        s_core_logger->set_level(static_cast<spdlog::level::level_enum>(level));
    }

    void Logger::set_client_level(LogLevel level) {
        s_client_logger->set_level(static_cast<spdlog::level::level_enum>(level));
    }

    void Logger::set_category_level(LogCategory category, LogLevel level) {
        if (auto it = s_category_loggers.find(category); it != s_category_loggers.end()) {
            it->second->set_level(static_cast<spdlog::level::level_enum>(level));
        }
    }

    std::string Logger::get_indent() {
        return std::string(LogScope::s_depth * 2, ' ');
    }

    LogScope::LogScope(const std::string_view name) : m_name(name) {
        Logger::get_core_logger()->info("{}╔═ {} START", Logger::get_indent(), m_name);
        s_depth++;
    }

    LogScope::~LogScope() {
        s_depth--;
        Logger::get_core_logger()->info("{}╚═ {} END", Logger::get_indent(), m_name);
    }
} // namespace star
