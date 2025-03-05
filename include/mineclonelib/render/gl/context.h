#pragma once

#include "mineclonelib/render/context.h"

#include <glm/glm.hpp>

namespace mc
{
namespace render
{
class gl_context : public context {
    public:
	gl_context(mc::window *window);
	virtual ~gl_context();

	virtual void begin() override;
	virtual void present() override;

    private:
	static void framebuffer_callback(glm::ivec2 prev_size, glm::ivec2 size);

	uint32_t m_framebuffer_callback;
};
}
}
