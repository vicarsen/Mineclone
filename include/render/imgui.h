#pragma once

#include "window.h"

#include <imgui.h>

namespace mc
{
  void imgui_init(window_t *wnd);
  void imgui_render();
  void imgui_present();
  void imgui_terminate();
};

