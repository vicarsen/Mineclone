#include "mineclonelib/log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

DEFINE_LOG_CATEGORY(Default);

namespace mc
{
::std::shared_ptr< ::spdlog::logger> create_logger(const char *name)
{
	auto logger = spdlog::stdout_color_mt(name);
	logger->set_level(spdlog::level::trace);
	logger->enable_backtrace(32);
	return logger;
}
}
