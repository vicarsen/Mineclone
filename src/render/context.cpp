#include "render/context.h"

#include <glad/gl.h>

DEFINE_LOG_CATEGORY(Render);

namespace mc
{
  namespace render
  {
    void context_create(context_t *ctx, window_t *wnd)
    {
      LOG_ASSERT(ctx != nullptr, Render, "Context pointer is null!");
      LOG_ASSERT(ctx->window == nullptr, Render, "Context is already created!");
      LOG_ASSERT(wnd != nullptr, Render, "Window pointer is null!");
      LOG_ASSERT(wnd->window != nullptr, Render, "Window was never created!");
      
      GLFWwindow *context = glfwGetCurrentContext();
      glfwMakeContextCurrent(wnd->window);
      LOG_ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), Render, "Failed to load OpenGL!");
      glfwMakeContextCurrent(context);

      ctx->window = wnd->window;
    }

    void context_make_current(context_t *ctx)
    {
      if (ctx == nullptr) {
        glfwMakeContextCurrent(nullptr);
      }

      LOG_ASSERT(ctx->window != nullptr, Render, "Context was never created!");
      glfwMakeContextCurrent(ctx->window);
    }

    void context_destroy(context_t *ctx)
    {
      LOG_ASSERT(ctx != nullptr, Render, "Context pointer is null!");
      LOG_WARN_IF(ctx->window == nullptr, Render, "Context was never created!");

      ctx->window = nullptr;
    }
  };
};

