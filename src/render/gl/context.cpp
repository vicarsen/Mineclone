#include "mineclonelib/render/gl/context.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace mc
{
namespace render
{
static void message_callback(GLenum source, GLenum type, GLuint id,
			     GLenum severity, GLsizei length,
			     const GLchar *message, const void *user_ptr)
{
	const char *src_str = "";
	switch (source) {
	case GL_DEBUG_SOURCE_API:
		src_str = "API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		src_str = "WINDOW_SYSTEM";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		src_str = "SHADER_COMPILER";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		src_str = "THIRD_PARTY";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		src_str = "APPLICATION";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		src_str = "OTHER";
		break;
	}

	const char *type_str = "";
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		type_str = "ERROR";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type_str = "DEPRECATED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type_str = "UNDEFINED_BEHAVIOR";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type_str = "PORTABILITY";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type_str = "PERFORMANCE";
		break;
	case GL_DEBUG_TYPE_MARKER:
		type_str = "MARKER";
		break;
	case GL_DEBUG_TYPE_OTHER:
		type_str = "OTHER";
		break;
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_TRACE(Render, "OpenGL {} {} ({}): {}", src_str, type_str,
			  id, message);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		LOG_INFO(Render, "OpenGL {} {} ({}): {}", src_str, type_str, id,
			 message);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_WARN(Render, "OpenGL {} {} ({}): {}", src_str, type_str, id,
			 message);
		break;
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_ERROR(Render, "OpenGL {} {} ({}): {}", src_str, type_str,
			  id, message);
		break;
	}
}

gl_context::gl_context(mc::window *window)
	: context(window)
{
	glfwMakeContextCurrent(m_window->get_handle());

	LOG_ASSERT(Render, gladLoadGL(glfwGetProcAddress),
		   "Failed to load OpenGL");

#ifndef NDEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(message_callback, nullptr);
#endif

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_MULTISAMPLE);

	glCullFace(GL_BACK);
	glFrontFace(GL_CW);

	glfwSwapInterval(0);
}

gl_context::~gl_context()
{
	if (glfwGetCurrentContext() == m_window->get_handle()) {
		glfwMakeContextCurrent(nullptr);
	}
}

void gl_context::begin(render_state *state)
{
	m_state = state;

	if (m_state->framebuffer_resized) {
		glViewport(0, 0, m_state->framebuffer.x,
			   m_state->framebuffer.y);

		m_state->framebuffer_resized = false;
	}

	glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_context::present()
{
	glfwSwapBuffers(m_window->get_handle());
}

void gl_context::make_current()
{
	glfwMakeContextCurrent(m_window->get_handle());
}

void gl_context::unmake_current()
{
	glfwMakeContextCurrent(nullptr);
}
}
}
