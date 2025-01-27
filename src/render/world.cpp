#include "render/world.h"

namespace mc
{
  namespace render
  {
    void world_create(world_t *world, chunk_mesh_generator_fn generator, usize lod)
    {
      LOG_ASSERT(world != nullptr, Render, "World pointer is null!");
      LOG_ASSERT(world->generator == nullptr, Render, "World was already created!");
    
      const usize max_chunk_size = sizeof(packed_chunk_quad_t) * 3 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2;
      const usize max_chunk_count = 56 * lod + 8;
      
      buffer_create(&world->faces, max_chunk_count * max_chunk_size);
      buffer_create(&world->ssbo, max_chunk_count * sizeof(world_t::ssbo_element_t));
      buffer_create(&world->indirect, 6 * max_chunk_count * sizeof(world_t::indirect_command_t));
      
      world->generator = generator;

      world->lods.resize(lod);
      
      usize idx = 0;
      for (usize j = 0; j < 64; j++) {
        world->lods[0].chunks[j].mesh = idx++;
        world->lods[0].chunks[j].coords = { -1'000'000'000, 0, 0 };
      }

      world->lods[0].origin = { -1'000'000'000, 0, 0 };

      for (usize i = 1; i < lod; i++) {
        for (usize j = 0; j < 56; j++) {
          world->lods[i].chunks[j].mesh = idx++;
          world->lods[i].chunks[j].coords = { -1'000'000'000, 0, 0 };
        }

        world->lods[i].origin = { -1'000'000'000, 0, 0 };
      }
    }

    static glm::ivec3 __divide(glm::ivec3 v, i32 div)
    {
      v.x = v.x / div - (v.x % div < 0);
      v.y = v.y / div - (v.y % div < 0);
      v.z = v.z / div - (v.z % div < 0);
      return v;
    }

    static glm::ivec3 __find_origin(glm::ivec3 position, usize lod)
    {
      i32 lod_size = (CHUNK_SIZE << lod);
      return __divide(position - lod_size / 2, lod_size) - 1;
    }

    static void __world_generate(world_t *world, glm::ivec3 coords, usize lod, usize idx)
    {
      thread_local static chunk_mesh_t mesh;
      
      const usize max_chunk_size = sizeof(packed_chunk_quad_t) * 3 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE / 2;

      world->generator(&mesh, coords, lod);

      usize top = world->lods[lod].chunks[idx].mesh * max_chunk_size / sizeof(packed_chunk_quad_t);
      usize cmd_idx = 6 * world->lods[lod].chunks[idx].mesh;
      
      auto __write = [&world, &top, &cmd_idx](const std::vector<packed_chunk_quad_t>& quads)
      {
        buffer_write(&world->faces,
                     top * sizeof(packed_chunk_quad_t),
                     quads.size() * sizeof(packed_chunk_quad_t),
                     quads.data());

        world_t::indirect_command_t command = {
          .count = 4,
          .instance_count = (u32)quads.size(),
          .first_vertex = 0,
          .base_instance = (u32)top
        };

        buffer_write(&world->indirect,
                     cmd_idx * sizeof(world_t::indirect_command_t),
                     sizeof(world_t::indirect_command_t),
                     &command);

        top += quads.size();
        cmd_idx++;
      };

      world_t::ssbo_element_t element = {
        .position = glm::ivec4(coords, lod)
      };

      buffer_write(&world->ssbo,
                   world->lods[lod].chunks[idx].mesh * sizeof(world_t::ssbo_element_t),
                   sizeof(world_t::ssbo_element_t),
                   &element);

      __write(mesh.east);
      __write(mesh.west);
      __write(mesh.up);
      __write(mesh.down);
      __write(mesh.north);
      __write(mesh.south);

      world->lods[lod].chunks[idx].coords = coords;
    }

    static void __world_update(world_t *world, glm::vec3 player, usize lod)
    {
      glm::ivec3 origin = __find_origin(player, lod);
      if (origin == world->lods[lod].origin) {
        return;
      }

      u64 mask = 0, chunks = (lod == 0 ? 64 : 56);
      for (usize i = 0; i < chunks; i++) {
        glm::ivec3 coords = world->lods[lod].chunks[i].coords - origin;
        if (0 <= coords.x && coords.x < 4 &&
            0 <= coords.y && coords.y < 4 &&
            0 <= coords.z && coords.z < 4) {
          mask |= (1ull << ((coords.x * 4 + coords.y) * 4 + coords.z));
        }
      }

      usize idx = 0;
      for (usize i = 0; i < 4; i++) {
        for (usize j = 0; j < 4; j++) {
          for (usize k = 0; k < 4; k++) {
            if (!(mask & (1 << ((i * 4 + j) * 4 + k)))) {
              glm::ivec3 coords = world->lods[lod].chunks[idx].coords - origin;
              while (0 <= coords.x && coords.x < 4 &&
                     0 <= coords.y && coords.y < 4 &&
                     0 <= coords.z && coords.z < 4) {
                coords = world->lods[lod].chunks[++idx].coords;
              }

              __world_generate(world, origin + glm::ivec3(i, j, k), lod, idx++);
            }
          }
        }
      }

      world->lods[lod].origin = origin;
    }

    void world_update(world_t *world, glm::vec3 player)
    {
      LOG_ASSERT(world != nullptr, Render, "World pointer is null!");
      LOG_ASSERT(world->generator != nullptr, Render, "World was never created!");

      for (usize lod = 0; lod < world->lods.size(); lod++) {
        __world_update(world, player, lod);
      }
    }

    void world_destroy(world_t *world)
    {
      LOG_ASSERT(world != nullptr, Render, "World pointer is null!");
      LOG_WARN_IF(world->generator != nullptr, Render, "World was never created!");
    
      buffer_destroy(&world->faces);
      buffer_destroy(&world->ssbo);
      buffer_destroy(&world->indirect);
      world->generator = nullptr;
      world->lods.clear();
    }
  };
};

