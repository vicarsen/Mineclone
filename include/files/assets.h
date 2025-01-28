#pragma once

#include <filesystem>

#include "log.h"
#include "types.h"

#ifndef ASSETS_PATH
#define ASSETS_PATH "./assets"
#endif

DECLARE_LOG_CATEGORY(Assets);

namespace mc
{
  namespace files
  {
    struct asset_t
    {
      std::filesystem::path path;
    };

    void asset_create(asset_t *asset, const char *filepath);
    std::vector<byte> asset_load_bin(asset_t *asset);
    void asset_destroy(asset_t *asset);
  };
};

