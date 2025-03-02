#pragma once

#include "mineclonelib/render/context.h"

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
};
}
}
