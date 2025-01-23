#pragma once

#include "types.h"

#include "render/context.h"

namespace mc
{
  namespace render
  {
    struct chunk_allocator_t
    {
      GLuint buffer = 0;
    };

    void chunk_allocator_create(chunk_allocator_t *allocator, usize chunk_size, usize chunks);
    void chunk_allocator_write(chunk_allocator_t *allocator, usize chunk, usize size, const void *data);
    void chunk_allocator_destroy(chunk_allocator_t *allocator);
  };
};

