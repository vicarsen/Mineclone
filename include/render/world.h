#pragma once

#include "types.h"

#include "world/chunk.h"
#include "render/chunk.h"

#include <glm/glm.hpp>

namespace mc
{
  namespace render
  {
    typedef void (*chunk_mesh_generator_fn)(chunk_mesh_t *mesh, glm::ivec3 coords, usize lod);

    struct world_t
    {
      struct lod_chunk_t
      {
        glm::ivec3 coords;
        usize mesh;
      };

      struct lod_t
      {
        lod_chunk_t chunks[64];
        glm::ivec3 origin;
      };

      chunk_allocator_t allocator;
      chunk_mesh_generator_fn generator;
      std::vector<lod_t> lods;
    };

    void world_create(world_t *world, chunk_mesh_generator_fn generator, usize lod);
    void world_update(world_t *world, glm::vec3 player);
    void world_destroy(world_t *world);
  };
};

