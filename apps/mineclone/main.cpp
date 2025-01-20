#include "window.h"

#include <glad/gl.h>

int main()
{
  mc::window_api_init();

  mc::window_t wnd;
  mc::window_create(&wnd, 1980, 1080, "Mineclone");

  glfwMakeContextCurrent(wnd.window);
  LOG_ASSERT(gladLoadGL((GLADloadfunc)glfwGetProcAddress), Default, "Failed to load OpenGL!");

  while (!mc::window_should_close(&wnd)) {
    mc::window_api_poll_events();

    glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(wnd.window);
  }

  mc::window_destroy(&wnd);
  mc::window_api_terminate();
  return 0;
}

