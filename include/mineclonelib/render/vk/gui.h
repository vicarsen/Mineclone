#pragma once

#include "mineclonelib/render/gui.h"

#include <glad/vulkan.h>

namespace mc
{
namespace render
{
class vk_gui_context : public gui_context {
    public:
	vk_gui_context(context *ctx);
	virtual ~vk_gui_context();

	virtual void begin() override;
	virtual void present() override;

    private:
	VkDescriptorPool m_pool;
};
}
}
