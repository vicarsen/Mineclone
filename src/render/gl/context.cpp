#include "mineclonelib/render/gl/context.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace mc
{
namespace render
{
gl_context::gl_context(mc::window *window)
	: context(window)
{
	glfwMakeContextCurrent(m_window->get_handle());

	LOG_ASSERT(Render, gladLoadGL(glfwGetProcAddress),
		   "Failed to load OpenGL");
}

gl_context::~gl_context()
{
	if (glfwGetCurrentContext() == m_window->get_handle()) {
		glfwMakeContextCurrent(nullptr);
	}
}

void gl_context::begin()
{
	glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void gl_context::present()
{
	glfwSwapBuffers(m_window->get_handle());
}
}
}
