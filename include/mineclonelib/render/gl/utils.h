#pragma once

#include <glad/gl.h>

namespace mc
{
namespace render
{
namespace gl
{
GLuint load_shader(const char *path, GLenum type);
bool check_compile_status(GLuint shader, const char *file = nullptr);
bool check_link_status(GLuint program);
}
}
}
