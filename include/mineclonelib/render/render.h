#pragma once

#include "mineclonelib/log.h"

#include <glm/glm.hpp>

DECLARE_LOG_CATEGORY(Render);

namespace mc
{
namespace render
{
enum class render_api { none = 0, opengl, vulkan };

struct render_state {
	glm::ivec2 framebuffer;
	bool framebuffer_resized;

	glm::mat4 view;
	glm::mat4 projection;
};
}
}
