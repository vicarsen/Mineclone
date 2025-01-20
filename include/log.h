#pragma once

#define LOG_LEVEL_TRACE SPDLOG_LEVEL_TRACE
#define LOG_LEVEL_DEBUG SPDLOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO SPDLOG_LEVEL_INFO
#define LOG_LEVEL_WARN SPDLOG_LEVEL_WARN
#define LOG_LEVEL_ERROR SPDLOG_LEVEL_ERROR
#define LOG_LEVEL_CRITICAL SPDLOG_LEVEL_CRITICAL

#ifndef MC_ACTIVE_LEVEL
#define MC_ACTIVE_LEVEL LOG_LEVEL_INFO
#endif

#define SPDLOG_ACTIVE_LEVEL MC_ACTIVE_LEVEL
#include <spdlog/spdlog.h>

#define DECLARE_LOG_CATEGORY(category) namespace mc { extern std::shared_ptr<spdlog::logger> __logger__##category; };
#define DEFINE_LOG_CATEGORY(category) namespace mc { std::shared_ptr<spdlog::logger> __logger__##category = logger_create(#category); };

#ifndef NDEBUG

#define LOG_TRACE(category, ...) SPDLOG_LOGGER_TRACE(mc::__logger__##category, __VA_ARGS__);
#define LOG_DEBUG(category, ...) SPDLOG_LOGGER_DEBUG(mc::__logger__##category, __VA_ARGS__);
#define LOG_INFO(category, ...) SPDLOG_LOGGER_INFO(mc::__logger__##category, __VA_ARGS__);
#define LOG_WARN(category, ...) SPDLOG_LOGGER_WARN(mc::__logger__##category, __VA_ARGS__);
#define LOG_ERROR(category, ...) SPDLOG_LOGGER_ERROR(mc::__logger__##category, __VA_ARGS__);
#define LOG_CRITICAL(category, ...) SPDLOG_LOGGER_CRITICAL(mc::__logger__##category, __VA_ARGS__);
#define LOG_ASSERT(condition, category, ...) if (!(condition)) { SPDLOG_LOGGER_CRITICAL(mc::__logger__##category, __VA_ARGS__); assert(0); }

#define LOG_TRACE_IF(condition, category, ...) if (condition) { SPDLOG_LOGGER_TRACE(mc::__logger__##category, __VA_ARGS__); }
#define LOG_DEBUG_IF(condition, category, ...) if (condition) { SPDLOG_LOGGER_DEBUG(mc::__logger__##category, __VA_ARGS__); }
#define LOG_INFO_IF(condition, category, ...) if (condition) { SPDLOG_LOGGER_INFO(mc::__logger__##category, __VA_ARGS__); }
#define LOG_WARN_IF(condition, category, ...) if (condition) { SPDLOG_LOGGER_WARN(mc::__logger__##category, __VA_ARGS__); }
#define LOG_ERROR_IF(condition, category, ...) if (condition) { SPDLOG_LOGGER_ERROR(mc::__logger__##category, __VA_ARGS__); }
#define LOG_CRITICAL_IF(condition, category, ...) if (condition) { SPDLOG_LOGGER_CRITICAL(mc::__logger__##category, __VA_ARGS__); }

#else

#define LOG_TRACE(category, ...)
#define LOG_DEBUG(category, ...)
#define LOG_INFO(category, ...)
#define LOG_WARN(category, ...)
#define LOG_ERROR(category, ...)
#define LOG_CRITICAL(category, ...)
#define LOG_ASSERT(condition, category, ...)

#define LOG_TRACE_IF(condition, category, ...)
#define LOG_DEBUG_IF(condition, category, ...)
#define LOG_INFO_IF(condition, category, ...)
#define LOG_WARN_IF(condition, category, ...)
#define LOG_ERROR_IF(condition, category, ...)
#define LOG_CRITICAL_IF(condition, category, ...)

#endif

DECLARE_LOG_CATEGORY(Default);

namespace mc
{
  std::shared_ptr<spdlog::logger> logger_create(const char* name);
};

