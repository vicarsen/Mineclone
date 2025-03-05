#pragma once

#include "mineclonelib/render/world.h"
#include "mineclonelib/world/chunk.h"

namespace mc
{
namespace render
{
class vk_world_renderer : public world_renderer {
    public:
	vk_world_renderer(context *ctx);
	virtual ~vk_world_renderer();

	virtual chunk_handle alloc_chunk() override;
	virtual void free_chunk(chunk_handle handle) override;

	virtual void upload_chunk(chunk_handle handle,
				  mc::world::chunk_draw_data *draw_data,
				  const glm::mat4 &model) override;

	virtual void render(const glm::mat4 &view,
			    const glm::mat4 &projection) override;
};
}
}
