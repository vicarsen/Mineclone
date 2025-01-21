#include <window.h>
#include <io.h>

#include <chrono>

int main()
{
  mc::window_api_init();

  mc::window_t wnd;
  mc::window_create(&wnd, 1280, 720, "Mineclone");

  mc::input_t input;
  mc::input_create(&input, &wnd);

  f32 start = input.time;
  
  std::this_thread::sleep_for(std::chrono::milliseconds(200));
  
  mc::input_preupdate(&input);
  mc::window_api_poll_events();
  mc::input_update(&input);

  f32 finish = input.time;
  f32 time = finish - start;

  f32 error = std::abs((time - 0.2f) / time);

  LOG_INFO(Default, "Time difference relative error: {}%.", error);
  LOG_ASSERT(error <= 0.02f, Default, "Time difference not within 2%!");

  mc::input_destroy(&input);

  mc::window_destroy(&wnd);
  mc::window_api_terminate();
  return 0;
}

