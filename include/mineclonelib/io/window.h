#pragma once

#include "mineclonelib/log.h"

DECLARE_LOG_CATEGORY(Window);

struct GLFWwindow;

namespace mc
{
class window {
    public:
	static void init();
	static void poll_events();
	static void terminate();

    public:
	window(const char *title, int width, int height);
	~window();

	bool should_close() const;

	inline GLFWwindow *get_handle() const noexcept
	{
		return m_window;
	}

    private:
	GLFWwindow *m_window;
};
}
