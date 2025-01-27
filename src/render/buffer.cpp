#include "render/context.h"
#include "render/buffer.h"

namespace mc
{
  namespace render
  {
    void buffer_create(buffer_t *buffer, usize size)
    {
      LOG_ASSERT(buffer != nullptr, Render, "Buffer pointer is null!");
      LOG_ASSERT(buffer->buffer != 0, Render, "Buffer is already created!");

      glCreateBuffers(1, &buffer->buffer);
      glNamedBufferStorage(buffer->buffer, size, NULL, GL_DYNAMIC_STORAGE_BIT);
    }

    void chunk_allocator_write(buffer_t *buffer, usize offset, usize size, const void *data)
    {
      LOG_ASSERT(buffer != nullptr, Render, "Buffer pointer is null!");
      LOG_ASSERT(buffer->buffer != 0, Render, "Buffer was never created!");

      glNamedBufferSubData(buffer->buffer, offset, size, data);
    }

    void buffer_destroy(buffer_t *buffer)
    {
      LOG_ASSERT(buffer != nullptr, Render, "Buffer pointer is null!");
      LOG_WARN_IF(buffer->buffer == 0, Render, "Buffer was never created!");

      glDeleteBuffers(1, &buffer->buffer);
      buffer->buffer = 0;
    }
  };
};

