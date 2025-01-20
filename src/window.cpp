#include "window.h"
#include "GLFW/glfw3.h"
#include "log.h"
#include <cstddef>

DEFINE_LOG_CATEGORY(Window);

namespace mc
{
  void window_api_init()
  {
    LOG_ASSERT(glfwInit(), Window, "Failed to initialize GLFW!");
  }

  void window_api_poll_events()
  {
    glfwPollEvents();
  }

  void window_api_terminate()
  {
    glfwTerminate();
  }

  void window_create(window_t *wnd, u32 width, u32 height, const char* title)
  {
    LOG_ASSERT(wnd != nullptr, Window, "Window pointer is null!");

    wnd->window = glfwCreateWindow(width, height, title, NULL, NULL);
    LOG_ASSERT(wnd->window != nullptr, Window, "Failed to create GLFW window!");
  }

  bool window_should_close(window_t *wnd)
  {
    return glfwWindowShouldClose(wnd->window);
  }

  void window_destroy(window_t *wnd)
  {
    LOG_ASSERT(wnd != nullptr, Window, "Window pointer is null!");
    LOG_WARN_IF(wnd->window == nullptr, Window, "Window was never created!");

    glfwDestroyWindow(wnd->window);
    wnd->window = nullptr;
  }
};

