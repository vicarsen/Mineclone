#include "mineclonelib/io/window.h"
#include "mineclonelib/io/input.h"
#include "mineclonelib/io/keys.h"
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
	m_window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (m_window == nullptr) {
		LOG_CRITICAL(Window, "Failed to create window");
	}

	glfwMakeContextCurrent(m_window);

	glfwSetWindowUserPointer(m_window, this);
	glfwSetKeyCallback(m_window, key_callback);
	glfwSetMouseButtonCallback(m_window, button_callback);
	glfwSetCursorPosCallback(m_window, cursor_callback);
	glfwSetScrollCallback(m_window, scroll_callback);
}

window::~window()
{
	glfwDestroyWindow(m_window);
}

void window::add_input_handler(input_handler *handler)
{
	m_input_handlers.emplace_back(handler);
}

void window::remove_input_handler(input_handler *handler)
{
	std::erase(m_input_handlers, handler);
}

bool window::should_close() const
{
	return glfwWindowShouldClose(m_window);
}

void window::key_callback(GLFWwindow *handle, int key, int scancode, int action,
			  int mods)
{
	window *wnd =
		reinterpret_cast<window *>(glfwGetWindowUserPointer(handle));

	input_event event;

	switch (action) {
	case GLFW_PRESS:
		event = input_event::pressed;
		break;
	case GLFW_RELEASE:
		event = input_event::released;
		break;
	}

	for (auto handler : wnd->m_input_handlers) {
		handler->key_callback(key, event);
	}
}

void window::button_callback(GLFWwindow *handle, int button, int action,
			     int mods)
{
	window *wnd =
		reinterpret_cast<window *>(glfwGetWindowUserPointer(handle));

	input_event event;

	switch (action) {
	case GLFW_PRESS:
		event = input_event::pressed;
		break;
	case GLFW_RELEASE:
		event = input_event::released;
		break;
	}

	for (auto handler : wnd->m_input_handlers) {
		handler->button_callback(button, event);
	}
}

void window::cursor_callback(GLFWwindow *handle, double x, double y)
{
	window *wnd =
		reinterpret_cast<window *>(glfwGetWindowUserPointer(handle));

	for (auto handler : wnd->m_input_handlers) {
		handler->cursor_callback(x, y);
	}
}

void window::scroll_callback(GLFWwindow *handle, double xoffset, double yoffset)
{
	window *wnd =
		reinterpret_cast<window *>(glfwGetWindowUserPointer(handle));

	for (auto handler : wnd->m_input_handlers) {
		handler->scroll_callback(xoffset, yoffset);
	}
}
}
