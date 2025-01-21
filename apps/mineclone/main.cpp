#include "cvar.h"
#include "window.h"
#include "io.h"
#include "render/context.h"

#include <glad/gl.h>

namespace cvars
{
  mc::cvar<u32> window_width("window.width", "The width of the window", 1920);
  mc::cvar<u32> window_height("window.height", "The height of the window", 1080);
  mc::cvar<const char*> window_title("window.title", "The title of the window", "Mineclone");
};

int main()
{
  mc::window_api_init();

  mc::window_t wnd;
  mc::window_create(&wnd, cvars::window_width.value, cvars::window_height.value, cvars::window_title.value);

  mc::input_t input;
  mc::input_create(&input, &wnd);

  mc::render::context_t ctx;
  mc::render::context_create(&ctx, &wnd);

  mc::render::context_make_current(&ctx);

  while (!mc::window_should_close(&wnd)) {
    mc::input_preupdate(&input);
    mc::window_api_poll_events();
    mc::input_update(&input);

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

