#pragma once

#define LOG_LEVEL_TRACE SPDLOG_LEVEL_TRACE
#define LOG_LEVEL_DEBUG SPDLOG_LEVEL_DEBUG
#define LOG_LEVEL_INFO SPDLOG_LEVEL_INFO
#define LOG_LEVEL_WARN SPDLOG_LEVEL_WARN
#define LOG_LEVEL_ERROR SPDLOG_LEVEL_ERROR
#define LOG_LEVEL_CRITICAL SPDLOG_LEVEL_CRITICAL
#define LOG_LEVEL_OFF SPDLOG_LEVEL_TRACE

#ifndef ACTIVE_LOG_LEVEL
#ifndef NDEBUG
#define ACTIVE_LOG_LEVEL LOG_LEVEL_INFO
#else
#define ACTIVE_LOG_LEVEL LOG_LEVEL_OFF
#endif
#endif

#define SPDLOG_ACTIVE_LEVEL ACTIVE_LOG_LEVEL
#include <spdlog/spdlog.h>

#define DECLARE_LOG_CATEGORY(category)                                    \
	namespace mc                                                      \
	{                                                                 \
	extern ::std::shared_ptr< ::spdlog::logger> __logger__##category; \
	}

#define DEFINE_LOG_CATEGORY(category)                               \
	namespace mc                                                \
	{                                                           \
	::std::shared_ptr< ::spdlog::logger> __logger__##category = \
		create_logger(#category);                           \
	}

#define LOG_TRACE(category, ...) \
	SPDLOG_LOGGER_TRACE(::mc::__logger__##category, __VA_ARGS__)

#define LOG_DEBUG(category, ...) \
	SPDLOG_LOGGER_DEBUG(::mc::__logger__##category, __VA_ARGS__)

#define LOG_INFO(category, ...) \
	SPDLOG_LOGGER_INFO(::mc::__logger__##category, __VA_ARGS__)

#define LOG_WARN(category, ...) \
	SPDLOG_LOGGER_WARN(::mc::__logger__##category, __VA_ARGS__)

#define LOG_ERROR(category, ...) \
	SPDLOG_LOGGER_ERROR(::mc::__logger__##category, __VA_ARGS__)

#define LOG_CRITICAL(category, ...) \
	SPDLOG_LOGGER_CRITICAL(::mc::__logger__##category, __VA_ARGS__)

#ifndef NDEBUG
#define LOG_ASSERT(category, condition, ...)                       \
	if (!(condition)) {                                        \
		SPDLOG_LOGGER_CRITICAL(::mc::__logger__##category, \
				       __VA_ARGS__);               \
		exit(1);                                           \
	}
#else
#define LOG_ASSERT(category, condition, ...)                       \
	if (!(condition)) {                                        \
		SPDLOG_LOGGER_CRITICAL(::mc::__logger__##category, \
				       __VA_ARGS__);               \
	}
#endif

#define LOG_TRACE_IF(category, condition, ...)    \
	if (condition) {                          \
		LOG_TRACE(category, __VA_ARGS__); \
	}

#define LOG_DEBUG_IF(category, condition, ...)    \
	if (condition) {                          \
		LOG_DEBUG(category, __VA_ARGS__); \
	}

#define LOG_INFO_IF(category, condition, ...)    \
	if (condition) {                         \
		LOG_INFO(category, __VA_ARGS__); \
	}

#define LOG_WARN_IF(category, condition, ...)    \
	if (condition) {                         \
		LOG_WARN(category, __VA_ARGS__); \
	}

#define LOG_ERROR_IF(category, condition, ...)    \
	if (condition) {                          \
		LOG_ERROR(category, __VA_ARGS__); \
	}

#define LOG_CRITICAL_IF(category, condition, ...)    \
	if (condition) {                             \
		LOG_CRITICAL(category, __VA_ARGS__); \
	}

DECLARE_LOG_CATEGORY(Default);

namespace mc
{
::std::shared_ptr< ::spdlog::logger> create_logger(const char *name);
}
