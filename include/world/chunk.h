#pragma once

#define CHUNK_SIZE 32
#define CHUNK_PADDING 1
#define CHUNK_FULL (CHUNK_SIZE + CHUNK_PADDING)

#include "types.h"

#include <vector>

namespace mc
{
  struct chunk_t
  {
    bool chunk[CHUNK_FULL][CHUNK_FULL][CHUNK_FULL];
  };

  typedef u16 packed_chunk_vertex_t;

  struct chunk_mesh_t
  {
    std::vector<packed_chunk_vertex_t> east;
    std::vector<packed_chunk_vertex_t> west;
    std::vector<packed_chunk_vertex_t> up;
    std::vector<packed_chunk_vertex_t> down;
    std::vector<packed_chunk_vertex_t> north;
    std::vector<packed_chunk_vertex_t> south;
  };

  void chunk_generate_mesh(chunk_t *chunk, chunk_mesh_t *mesh);
};

