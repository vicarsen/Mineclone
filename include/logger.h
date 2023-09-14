#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include <spdlog/spdlog.h>

#include "fmt_glm.h"

#define DECLARE_LOG_CATEGORY(name) extern std::shared_ptr<spdlog::logger> CategoryLogger##name;
#define DEFINE_LOG_CATEGORY(name, ...) std::shared_ptr<spdlog::logger> CategoryLogger##name = Logger::CreateLogger(#name, __VA_ARGS__);

#define TRACE(category, ...) SPDLOG_LOGGER_TRACE(CategoryLogger##category, __VA_ARGS__);
#define INFO(category, ...) SPDLOG_LOGGER_INFO(CategoryLogger##category, __VA_ARGS__);
#define WARN(category, ...) SPDLOG_LOGGER_WARN(CategoryLogger##category, __VA_ARGS__);
#define ERROR(category, ...) SPDLOG_LOGGER_ERROR(CategoryLogger##category, __VA_ARGS__);
#define CRITICAL(category, ...) SPDLOG_LOGGER_CRITICAL(CategoryLogger##category, __VA_ARGS__);

DECLARE_LOG_CATEGORY(LogTemp);

namespace Logger
{
    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name);
    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const spdlog::level::level_enum& level);
    std::shared_ptr<spdlog::logger> CreateLogger(const std::string& name, const spdlog::level::level_enum& level, const std::string& file);
};

