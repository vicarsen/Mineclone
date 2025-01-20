#include <window.h>
#include <render/context.h>

int main()
{
  mc::window_api_init();

  mc::window_t wnd;
  mc::window_create(&wnd, 1280, 720, "Mineclone");

  mc::render::context_t ctx;
  mc::render::context_create(&ctx, &wnd);
  
  mc::render::context_make_current(&ctx);

  mc::render::context_destroy(&ctx);
  mc::window_destroy(&wnd);
  mc::window_api_terminate();
  return 0;
}

