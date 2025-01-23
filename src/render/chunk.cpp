#include "render/chunk.h"
#include "render/context.h"

namespace mc
{
  namespace render
  {
    void chunk_allocator_create(chunk_allocator_t *allocator, usize chunk_size, usize chunks)
    {
      LOG_ASSERT(allocator != nullptr, Render, "Chunk allocator pointer is null!");
      LOG_ASSERT(allocator->buffer != 0, Render, "Chunk allocator is already created!");

      glCreateBuffers(1, &allocator->buffer);
      glNamedBufferStorage(allocator->buffer, chunk_size * chunks, NULL, GL_DYNAMIC_STORAGE_BIT);
    }

    void chunk_allocator_write(chunk_allocator_t *allocator, usize chunk, usize size, const void *data)
    {
      LOG_ASSERT(allocator != nullptr, Render, "Chunk allocator pointer is null!");
      LOG_ASSERT(allocator->buffer != 0, Render, "Chunk allocator was never created!");

      glNamedBufferSubData(allocator->buffer, chunk, size, data);
    }

    void chunk_allocator_destroy(chunk_allocator_t *allocator)
    {
      LOG_ASSERT(allocator != nullptr, Render, "Chunk allocator pointer is null!");
      LOG_WARN_IF(allocator->buffer == 0, Render, "Chunk allocator was never created!");

      glDeleteBuffers(1, &allocator->buffer);
      allocator->buffer = 0;
    }
  };
};

