#include "mineclonelib/io/window.h"
#include "mineclonelib/log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

DEFINE_LOG_CATEGORY(Window);

namespace mc
{
void window::init()
{
	LOG_ASSERT(Window, glfwInit(), "Failed to initialize GLFW");
}

void window::poll_events()
{
	glfwPollEvents();
}

void window::terminate()
{
	glfwTerminate();
}

window::window(const char *title, int width, int height)
{
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (m_window == nullptr) {
		LOG_CRITICAL(Window, "Failed to create window");
	}

	glfwSetWindowUserPointer(m_window, this);
}

window::~window()
{
	glfwDestroyWindow(m_window);
}

bool window::should_close() const
{
	return glfwWindowShouldClose(m_window);
}
}
