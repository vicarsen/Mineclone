#pragma once

#include "logger.h"

#include "utils/string.h"

#ifdef NDEBUG
#define PATH(asset) "../assets/" asset
#define LOGFILE(log) "../logs/" log
#else
#define PATH(asset) "../assets/" asset
#define LOGFILE(log) "../logs/" log
#endif

DECLARE_LOG_CATEGORY(Files);

namespace Files
{
    ::Utils::String ReadFile(const char* path);
};

