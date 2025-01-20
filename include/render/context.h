#pragma once

#include "log.h"
#include "window.h"

DECLARE_LOG_CATEGORY(Render);

namespace mc
{
  namespace render
  {
    struct context_t
    {
      GLFWwindow *window = nullptr;
    };

    void context_create(context_t *ctx, window_t *wnd);
    void context_make_current(context_t *ctx);
    void context_destroy(context_t *ctx);
  };
};

