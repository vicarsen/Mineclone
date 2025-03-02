#pragma once

#include <memory>

#include "mineclonelib/io/window.h"

namespace mc
{
namespace render
{
class context {
    public:
	context(window *wnd);
	virtual ~context() = default;

	virtual void begin() = 0;
	virtual void present() = 0;

	inline window *get_window() const noexcept
	{
		return m_window;
	}

	static std::unique_ptr<context> create(mc::window *window);

    protected:
	window *m_window;
};
}
}
