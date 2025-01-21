#include "window.h"
#include "io.h"
#include "render/context.h"

#include <glad/gl.h>

int main()
{
  mc::window_api_init();

  mc::window_t wnd;
  mc::window_create(&wnd, 1980, 1080, "Mineclone");

  mc::input_t input;
  mc::input_create(&input, &wnd);

  mc::render::context_t ctx;
  mc::render::context_create(&ctx, &wnd);

  mc::render::context_make_current(&ctx);

  while (!mc::window_should_close(&wnd)) {
    mc::input_preupdate(&input);
    mc::window_api_poll_events();
    mc::input_update(&input);

    LOG_INFO_IF(input.key_pressed[GLFW_KEY_A], Default, "A is pressed.");
    LOG_INFO_IF(input.key_just_pressed[GLFW_KEY_S], Default, "S has just been pressed.");
    LOG_INFO_IF(input.key_just_released[GLFW_KEY_D], Default, "D has just been released.");

    glClearColor(0.1f, 0.2f, 0.8f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(wnd.window);
  }

  mc::render::context_destroy(&ctx);

  mc::input_destroy(&input);

  mc::window_destroy(&wnd);
  mc::window_api_terminate();
  return 0;
}

