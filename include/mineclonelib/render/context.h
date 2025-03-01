#pragma once

#include <memory>

#include "mineclonelib/log.h"
#include "mineclonelib/io/window.h"

DECLARE_LOG_CATEGORY(Render);

namespace mc
{
namespace render
{
enum class render_api { none = 0, opengl, vulkan };

class context {
    public:
	virtual ~context() = default;

	virtual void begin() = 0;
	virtual void present() = 0;

	static std::unique_ptr<context> create(render_api api,
					       mc::window *window);
};
}
}
