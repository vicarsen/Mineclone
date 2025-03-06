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

	virtual void begin(render_state *state) override;
	virtual void present() override;

	virtual void make_current() override;
	virtual void unmake_current() override;
};
}
}
