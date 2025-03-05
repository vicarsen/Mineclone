
set(
  HEADER_LIST
  "../include/mineclonelib/log.h"
  "../include/mineclonelib/cvar.h"
  "../include/mineclonelib/misc.h"
  "../include/mineclonelib/transform.h"
  "../include/mineclonelib/application.h"
  "../include/mineclonelib/entrypoint.h"

  "../include/mineclonelib/world/blocks.h"
  "../include/mineclonelib/world/chunk.h"

  "../include/mineclonelib/io/assets.h"
  "../include/mineclonelib/io/keys.h"
  "../include/mineclonelib/io/input.h"
  "../include/mineclonelib/io/window.h"

  "../include/mineclonelib/render/context.h"
  "../include/mineclonelib/render/gui.h"
  "../include/mineclonelib/render/world.h"

  "../include/mineclonelib/render/gl/utils.h"
  "../include/mineclonelib/render/gl/context.h"
  "../include/mineclonelib/render/gl/gui.h"
  "../include/mineclonelib/render/gl/world.h"

  "../include/mineclonelib/render/vk/context.h"
  "../include/mineclonelib/render/vk/gui.h"
  "../include/mineclonelib/render/vk/world.h"

  "../include/mineclonelib/mineclone.h"
)

add_library(
  mineclonelib

  log.cpp
  cvar.cpp
  transform.cpp
  application.cpp

  world/blocks.cpp
  world/chunk.cpp

  io/input.cpp
  io/window.cpp

  render/context.cpp
  render/gui.cpp
  render/world.cpp

  render/gl/utils.cpp
  render/gl/context.cpp
  render/gl/gui.cpp
  render/gl/world.cpp

  render/vk/context.cpp
  render/vk/gui.cpp
  render/vk/world.cpp

  ${HEADER_LIST}
)

target_include_directories(mineclonelib PUBLIC ../include)
target_link_libraries(mineclonelib PUBLIC spdlog glm stb_image glfw glad imgui imgui_glfw imgui_opengl3 imgui_vulkan)
target_compile_features(mineclonelib PUBLIC cxx_std_20)

if (MINECLONE_DEBUG)
  target_compile_definitions(mineclonelib PUBLIC ASSETS_PATH=\"${Mineclone_SOURCE_DIR}/assets/\")
else()
  target_compile_definitions(mineclonelib PUBLIC ASSETS_PATH=\"./assets/\")
endif()

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include/mineclonelib"
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)

