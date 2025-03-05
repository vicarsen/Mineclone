#include "mineclonelib/render/world.h"
#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gl/world.h"
#include "mineclonelib/render/vk/world.h"

namespace mc
{
namespace render
{
world_renderer::world_renderer(context *ctx)
	: m_ctx(ctx)
{
}

std::unique_ptr<world_renderer> world_renderer::create(context *ctx)
{
	render_api api = ctx->get_window()->get_api();

	switch (api) {
	case render_api::opengl:
		return std::make_unique<gl_world_renderer>(ctx);
	case render_api::vulkan:
		return std::make_unique<vk_world_renderer>(ctx);
	case render_api::none:
		LOG_CRITICAL(
			Render,
			"Failed to create world_renderer: render_api::none currently not supported");
		return nullptr;
	default:
		return nullptr;
	}
}
}
}
