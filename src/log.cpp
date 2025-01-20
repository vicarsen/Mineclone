#include "log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

DEFINE_LOG_CATEGORY(Default);

namespace mc
{
  std::shared_ptr<spdlog::logger> logger_create(const char* name)
  {
    return spdlog::stdout_color_mt(name);
  }
};

