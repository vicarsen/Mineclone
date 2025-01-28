#include "files/assets.h"

#include <cstdio>

DEFINE_LOG_CATEGORY(Assets);

namespace mc
{
  namespace files
  {
    void asset_create(asset_t *asset, const char *filepath)
    {
      LOG_ASSERT(asset != nullptr, Assets, "Asset pointer is null!");

      asset->path = std::filesystem::path(ASSETS_PATH) / filepath;
    }

    std::vector<byte> asset_load_bin(asset_t *asset)
    {
      LOG_ASSERT(asset != nullptr, Assets, "Asset pointer is null!");

      FILE *file = fopen(asset->path.c_str(), "rb");
      if (file == nullptr) {
        LOG_ERROR(Assets, "Failed to load asset {}!", asset->path.string());
        return {};
      }

      fseek(file, 0, SEEK_END);
      auto length = ftell(file);
      fseek(file, 0, SEEK_SET);
    
      std::vector<byte> bin(length);
      if (fread(bin.data(), sizeof(byte), length, file) != length) {
        LOG_ERROR(Assets, "Failed to load asset {}!", asset->path.string());

        fclose(file);
        return {};
      }

      fclose(file);
      return bin;
    }

    void asset_destroy(asset_t *asset)
    {
      LOG_ASSERT(asset != nullptr, Assets, "Asset pointer is null!");
      LOG_WARN_IF(asset->path.empty(), Assets, "Asset was never created!");
    
      asset->path.clear();
    }
  };
};

