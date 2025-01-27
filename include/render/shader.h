#pragma once

#include <string_view>

#include <glad/gl.h>

namespace mc
{
  namespace render
  {
    struct shader_t
    {
      GLuint program;
    };

    void shader_create(shader_t *shader, std::string_view vertex_src, std::string_view fragment_src);
    void shader_destroy(shader_t *shader);
  };
};

