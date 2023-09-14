#pragma once

#include <string>

#include "logger.h"

#ifdef NDEBUG
#define PATH(asset) "./assets/" asset
#define LOGFILE(log) "./logs/" log
#else
#define PATH(asset) "../assets/" asset
#define LOGFILE(log) "../logs/" log
#endif

DECLARE_LOG_CATEGORY(Files);

namespace Files
{
    std::string ReadFile(const std::string& path);
};

