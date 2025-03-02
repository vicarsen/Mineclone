#include "mineclonelib/render/context.h"
#include "mineclonelib/render/gl/context.h"
#include "mineclonelib/render/vk/context.h"

DEFINE_LOG_CATEGORY(Render);

namespace mc
{
namespace render
{
std::unique_ptr<context> context::create(mc::window *window)
{
	switch (window->get_api()) {
	case render_api::opengl:
		return std::make_unique<gl_context>(window);
	case render_api::vulkan:
		return std::make_unique<vk_context>(window);
	case render_api::none:
		LOG_CRITICAL(
			Render,
			"Failed to create context: render_api::none currently not supported");
		return nullptr;
	default:
		return nullptr;
	}
}
}
}
