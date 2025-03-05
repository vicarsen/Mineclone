#pragma once

#include "mineclonelib/render/context.h"

#include "mineclonelib/world/chunk.h"

#include <glm/glm.hpp>

namespace mc
{
namespace render
{
using chunk_handle = uint32_t;

class world_renderer {
    public:
	world_renderer(context *ctx);
	virtual ~world_renderer() = default;

	virtual chunk_handle alloc_chunk() = 0;
	virtual void free_chunk(chunk_handle handle) = 0;

	virtual void upload_chunk(chunk_handle handle,
				  mc::world::chunk_draw_data *draw_data,
				  const glm::mat4 &model) = 0;

	virtual void render(const glm::mat4 &view,
			    const glm::mat4 &projection) = 0;

	static std::unique_ptr<world_renderer> create(context *ctx);

    protected:
	context *m_ctx;
};
}
}
