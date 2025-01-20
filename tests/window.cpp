#include <window.h>

int main()
{
  mc::window_api_init();

  mc::window_t wnd;
  mc::window_create(&wnd, 1280, 720, "Mineclone");

  mc::window_destroy(&wnd);
  mc::window_api_terminate();
  return 0;
}

