#include "mineclonelib/render/vk/world.h"
#include "mineclonelib/render/world.h"
#include "mineclonelib/world/chunk.h"

namespace mc
{
namespace render
{
vk_world_renderer::vk_world_renderer(context *ctx)
	: world_renderer(ctx)
{
}

vk_world_renderer::~vk_world_renderer()
{
}

chunk_handle vk_world_renderer::alloc_chunk()
{
	return 0;
}

void vk_world_renderer::free_chunk(chunk_handle handle)
{
}

void vk_world_renderer::upload_chunk(chunk_handle handle,
				     mc::world::chunk_draw_data *draw_data,
				     const glm::mat4 &model)
{
}

void vk_world_renderer::render(const glm::mat4 &view,
			       const glm::mat4 &projection)
{
}
}
}
