#pragma once

#include "types.h"

#include "world/chunk.h"
#include "render/buffer.h"

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

      struct ssbo_element_t
      {
        glm::ivec4 position;
      };

      struct indirect_command_t
      {
        GLuint count;
        GLuint instance_count;
        GLuint first_vertex;
        GLuint base_instance;
      };

      buffer_t faces, ssbo, indirect;
      chunk_mesh_generator_fn generator;
      std::vector<lod_t> lods;
    };

    void world_create(world_t *world, chunk_mesh_generator_fn generator, usize lod);
    void world_update(world_t *world, glm::vec3 player);
    void world_destroy(world_t *world);
  };
};

