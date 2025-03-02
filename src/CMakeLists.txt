
set(
  HEADER_LIST
  "../include/mineclonelib/log.h"
  "../include/mineclonelib/cvar.h"
  "../include/mineclonelib/misc.h"

  "../include/mineclonelib/io/keys.h"
  "../include/mineclonelib/io/input.h"
  "../include/mineclonelib/io/window.h"

  "../include/mineclonelib/render/context.h"
  "../include/mineclonelib/render/gui.h"

  "../include/mineclonelib/render/gl/context.h"
  "../include/mineclonelib/render/gl/gui.h"

  "../include/mineclonelib/render/vk/context.h"
  "../include/mineclonelib/render/vk/gui.h"
)

add_library(
  mineclonelib

  log.cpp
  cvar.cpp

  io/input.cpp
  io/window.cpp

  render/context.cpp
  render/gui.cpp

  render/gl/context.cpp
  render/gl/gui.cpp

  render/vk/context.cpp
  render/vk/gui.cpp

  ${HEADER_LIST}
)

target_include_directories(mineclonelib PUBLIC ../include)
target_link_libraries(mineclonelib PUBLIC spdlog glm glfw glad imgui imgui_glfw imgui_opengl3 imgui_vulkan)
target_compile_features(mineclonelib PUBLIC cxx_std_20)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include/mineclonelib"
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)

