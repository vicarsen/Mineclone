#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#define DECLARE_LOG_CATEGORY(name) extern ::std::shared_ptr<::spdlog::logger> CategoryLogger##name;
#define DEFINE_LOG_CATEGORY(name, desc) ::std::shared_ptr<::spdlog::logger> CategoryLogger##name = ::Logger::CreateLogger(#name, desc);

#define CONSOLE_LOGGER(_level) ::Logger::LoggerDesc { .type = ::Logger::LoggerDesc::CONSOLE, .level = ::spdlog::level::_level, .console_logger = ::Logger::ConsoleLoggerDesc{} }
#define FILE_LOGGER(_level, _file_path) ::Logger::LoggerDesc { .type = ::Logger::LoggerDesc::FILE, .level = ::spdlog::level::_level, .file_logger = ::Logger::FileLoggerDesc{ .file_path = _file_path } }

#define TRACE(category, ...) SPDLOG_LOGGER_TRACE(CategoryLogger##category, __VA_ARGS__);
#define INFO(category, ...) SPDLOG_LOGGER_INFO(CategoryLogger##category, __VA_ARGS__);
#define WARN(category, ...) SPDLOG_LOGGER_WARN(CategoryLogger##category, __VA_ARGS__);
#define ERROR(category, ...) SPDLOG_LOGGER_ERROR(CategoryLogger##category, __VA_ARGS__);
#define CRITICAL(category, ...) SPDLOG_LOGGER_CRITICAL(CategoryLogger##category, __VA_ARGS__);

DECLARE_LOG_CATEGORY(LogTemp);

namespace Logger
{
    struct ConsoleLoggerDesc
    {
    };

    struct FileLoggerDesc
    {
        const char* file_path;
    };

    struct LoggerDesc
    {
        enum Type { CONSOLE, FILE };

        Type type;
        ::spdlog::level::level_enum level;

        union
        {
            ConsoleLoggerDesc console_logger;
            FileLoggerDesc file_logger;
        };
    };

    ::std::shared_ptr<::spdlog::logger> CreateLogger(const char* name, const LoggerDesc& desc);
};

