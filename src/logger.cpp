#include "logger.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

DEFINE_LOG_CATEGORY(LogTemp, spdlog::level::trace);

namespace Logger
{
    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name)
    {
        auto logger = spdlog::stdout_color_mt(name);
        logger->set_level(spdlog::level::trace);
        return logger;
    }

    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const spdlog::level::level_enum& level)
    {
        auto logger = spdlog::stdout_color_mt(name);
        logger->set_level(level);
        return logger;
    }

    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const spdlog::level::level_enum& level, const std::string& file)
    {
#ifdef NDEBUG
        auto logger = spdlog::stdout_color_mt(name);
#else
        auto logger = spdlog::basic_logger_mt(name, file);
#endif
        logger->set_level(level);
        return logger;
    }
};

