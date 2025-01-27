#include "render/context.h"
#include "render/shader.h"

namespace mc
{
  namespace render
  {
    static GLuint __shader_check_status(GLuint shader)
    {
      GLint status;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
      if (status != GL_TRUE) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);
        
        std::vector<char> message(log_length);
        glGetShaderInfoLog(shader, log_length, NULL, message.data());
        
        LOG_ERROR(Render, "Failed to compile shader! Info log:\n{}", message.data());
        
        glDeleteShader(shader);
        return 0;
      }

      return shader;
    }

    static GLuint __shader_create(std::string_view src, GLenum type)
    {
      GLuint shader = glCreateShader(type);

      const GLchar *__src = src.data();
      GLint length = src.length();
      glShaderSource(shader, 1, &__src, &length);

      glCompileShader(shader);

      return __shader_check_status(shader);
    }

    static GLuint __shader_create(const void *binary, usize length, GLenum type)
    {
      GLuint shader = glCreateShader(type);
      glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, binary, length);
      glSpecializeShader(shader, "main", 0, 0, 0);
      return __shader_check_status(shader);
    }

    static GLuint __program_create(GLuint vertex_shader, GLuint fragment_shader)
    {
      GLuint program = glCreateProgram();

      glAttachShader(program, vertex_shader);
      glAttachShader(program, fragment_shader);

      glLinkProgram(program);

      glDetachShader(program, vertex_shader);
      glDetachShader(program, fragment_shader);
    
      glDeleteShader(vertex_shader);
      glDeleteShader(fragment_shader);

      GLint status;
      glGetProgramiv(program, GL_LINK_STATUS, &status);
      if (status != GL_TRUE) {
        GLint log_length;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);

        std::vector<char> message(log_length);
        glGetProgramInfoLog(program, log_length, NULL, message.data());

        LOG_ERROR(Render, "Failed to link shader program:\n{}", message.data());

        glDeleteProgram(program);
        return 0;
      }

      return program;
    }

    void shader_create(shader_t *shader, std::string_view vertex_src, std::string_view fragment_src)
    {
      LOG_ASSERT(shader != nullptr, Render, "Shader pointer is null!");
      LOG_ASSERT(shader->program == 0, Render, "Shader was already created!");
      LOG_ASSERT(!vertex_src.empty(), Render, "Vertex source is empty!");
      LOG_ASSERT(!fragment_src.empty(), Render, "Fragment source is empty!");
      
      GLuint vertex_shader = __shader_create(vertex_src, GL_VERTEX_SHADER);
      GLuint fragment_shader = __shader_create(fragment_src, GL_FRAGMENT_SHADER);
    
      shader->program = __program_create(vertex_shader, fragment_shader);
    }

    void shader_create(shader_t *shader,
                       const void *vertex_binary, usize vertex_length,
                       const void *fragment_binary, usize fragment_length)
    {
      LOG_ASSERT(shader != nullptr, Render, "Shader pointer is null!");
      LOG_ASSERT(shader->program == 0, Render, "Shader was already created!");
      LOG_ASSERT(vertex_binary != nullptr, Render, "Vertex binary is null!");
      LOG_ASSERT(fragment_binary != nullptr, Render, "Fragment binary is null!");
    
      GLuint vertex_shader = __shader_create(vertex_binary, vertex_length, GL_VERTEX_SHADER);
      GLuint fragment_shader = __shader_create(fragment_binary, fragment_length, GL_FRAGMENT_SHADER);

      shader->program = __program_create(vertex_shader, fragment_shader);
    }

    void shader_destroy(shader_t *shader)
    {
      LOG_ASSERT(shader != nullptr, Render, "Shader pointer is null!");
      LOG_WARN_IF(shader->program == 0, Render, "Shader was never created!");

      glDeleteProgram(shader->program);
      shader->program = 0;
    }
  };
};

