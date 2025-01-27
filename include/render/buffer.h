#pragma once

#include "types.h"

#include "render/context.h"

namespace mc
{
  namespace render
  {
    struct buffer_t 
    {
      GLuint buffer = 0;
    };

    void buffer_create(buffer_t *buffer, usize size);
    void buffer_write(buffer_t *buffer, usize offset, usize size, const void *data);
    void buffer_destroy(buffer_t *buffer);
  };
};

