#pragma once

#include "mineclonelib/log.h"
#include "mineclonelib/render/render.h"

DECLARE_LOG_CATEGORY(Window);

struct GLFWwindow;

namespace mc
{
class input_handler;

class window {
    public:
	static void init();
	static void poll_events();
	static void terminate();

    public:
	window(const char *title, int width, int height);
	~window();

	void add_input_handler(input_handler *handler);
	void remove_input_handler(input_handler *handler);

	bool should_close() const;

	inline render::render_api get_api() const noexcept
	{
		return m_api;
	}

	inline GLFWwindow *get_handle() const noexcept
	{
		return m_window;
	}

    private:
	static void framebuffer_callback(GLFWwindow *window, int width,
					 int height);
	static void key_callback(GLFWwindow *window, int key, int scancode,
				 int action, int mods);
	static void button_callback(GLFWwindow *window, int button, int action,
				    int mods);
	static void cursor_callback(GLFWwindow *window, double x, double y);
	static void scroll_callback(GLFWwindow *window, double xoffset,
				    double yoffset);

    private:
	render::render_api m_api;
	GLFWwindow *m_window;
	std::vector<input_handler *> m_input_handlers;
};
}
