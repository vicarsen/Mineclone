#include "mineclonelib/render/gl/utils.h"
#include "mineclonelib/render/render.h"

#include <fstream>
#include <sstream>
#include <vector>

namespace mc
{
namespace render
{
namespace gl
{
static std::string read_file(const char *path)
{
	std::ifstream file(path);
	if (!file) {
		LOG_ERROR(Render, "Failed to open shader source file {}", path);
		return "";
	}

	std::stringstream ss;
	ss << file.rdbuf();
	return ss.str();
}

GLuint load_shader(const char *path, GLenum type)
{
	std::string src = read_file(path);
	if (src.empty()) {
		return 0;
	}

	const GLchar *c_src = src.c_str();

	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &c_src, NULL);
	glCompileShader(shader);

	if (!check_compile_status(shader)) {
		glDeleteShader(shader);
		shader = 0;
	}

	return shader;
}

bool check_compile_status(GLuint shader, const char *file)
{
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE) {
		return true;
	}

	GLint length;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);

	std::vector<char> message(length + 1);
	glGetShaderInfoLog(shader, length, &length, message.data());

	if (file != nullptr) {
		LOG_ERROR(Render, "Failed to compile OpenGL shader {}!", file);
	} else {
		LOG_ERROR(Render, "Failed to compile OpenGL shader!");
	}

	LOG_ERROR(Render, "OpenGL Info Log:\n{}", message.data());
	return false;
}

bool check_link_status(GLuint program)
{
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_TRUE) {
		return true;
	}

	GLint length;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);

	std::vector<char> message(length + 1);
	glGetProgramInfoLog(program, length, &length, message.data());

	LOG_ERROR(Render, "Failed to link OpenGL program!");
	LOG_ERROR(Render, "OpenGL Info Log:\n{}", message.data());
	return false;
}
}
}
}
