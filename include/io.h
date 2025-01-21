#pragma once

#include "log.h"
#include "window.h"

#include <glm/glm.hpp>

#include <bitset>

DECLARE_LOG_CATEGORY(IO);

namespace mc
{
  struct input_t
  {
    window_t *window = nullptr;
    std::bitset<GLFW_KEY_LAST + 1> key_pressed;
    std::bitset<GLFW_KEY_LAST + 1> key_just_pressed;
    std::bitset<GLFW_KEY_LAST + 1> key_just_released;
    std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> button_pressed;
    std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> button_just_pressed;
    std::bitset<GLFW_MOUSE_BUTTON_LAST + 1> button_just_released;
    glm::vec2 last_cursor_pos, cursor_pos, cursor_delta;
    glm::vec2 scroll;
    f32 last_time, time, delta_time;
  };

  void input_create(input_t *input, window_t *wnd);
  void input_preupdate(input_t *input);
  void input_update(input_t *input);
  void input_destroy(input_t *input);
};

