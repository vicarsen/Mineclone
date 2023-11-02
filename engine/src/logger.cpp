#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

DEFINE_LOG_CATEGORY(LogTemp, CONSOLE_LOGGER(trace));

namespace Logger
{
    static std::shared_ptr<spdlog::logger> CreateConsoleLogger(const char* name, const ConsoleLoggerDesc& desc);
    static std::shared_ptr<spdlog::logger> CreateFileLogger(const char* name, const FileLoggerDesc& desc);

    std::shared_ptr<spdlog::logger> CreateLogger(const char* name, const LoggerDesc& desc)
    {
        std::shared_ptr<spdlog::logger> logger;

        switch(desc.type)
        {
        case LoggerDesc::CONSOLE: logger = CreateConsoleLogger(name, desc.console_logger); break;
        case LoggerDesc::FILE: logger = CreateFileLogger(name, desc.file_logger); break;
        }

        logger->set_level(desc.level);
        
        return logger;
    }

    static std::shared_ptr<spdlog::logger> CreateConsoleLogger(const char* name, const ConsoleLoggerDesc& desc)
    {
        std::shared_ptr<spdlog::logger> logger = spdlog::stdout_color_mt(name);
        return logger;
    }

    static std::shared_ptr<spdlog::logger> CreateFileLogger(const char* name, const FileLoggerDesc& desc)
    {
        std::shared_ptr<spdlog::logger> logger = spdlog::basic_logger_mt(name, desc.file_path);
        return logger;
    }
};

