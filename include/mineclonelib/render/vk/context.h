#pragma once

#include "mineclonelib/render/context.h"

namespace mc
{
namespace render
{
class vk_context : public context {
    public:
	vk_context(mc::window *window);
	virtual ~vk_context();

	virtual void begin() override;
	virtual void present() override;
};
}
}
