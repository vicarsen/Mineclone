#pragma once

#include "types.h"
#include "log.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

DECLARE_LOG_CATEGORY(Window);

namespace mc
{
  struct window_t
  {
    GLFWwindow *window = nullptr;
  };

  void window_api_init();
  void window_api_poll_events();
  void window_api_terminate();

  void window_create(window_t *wnd, u32 width, u32 height, const char *title);
  bool window_should_close(window_t *wnd);
  void window_destroy(window_t *wnd);
};

