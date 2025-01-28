
add_library(
  MinecloneLib

  ../include/cvar.h
  ../include/profiler.h

  log.cpp
  ../include/log.h

  window.cpp
  ../include/window.h

  io.cpp
  ../include/io.h

  files/assets.cpp
  ../include/files/assets.h

  render/context.cpp
  ../include/render/context.h

  render/imgui.cpp
  ../include/render/imgui.h

  world/chunk.cpp
  ../include/world/chunk.h

  render/buffer.cpp
  ../include/render/buffer.h

  render/shader.cpp
  ../include/render/shader.h

  render/world.cpp
  ../include/render/world.h
)

target_include_directories(MinecloneLib PUBLIC ../include/)
target_link_libraries(MinecloneLib PUBLIC spdlog glm glfw glad imgui imgui_glfw imgui_opengl3)
target_compile_features(MinecloneLib PUBLIC cxx_std_20)

if (MINECLONE_DEBUG)
  target_compile_definitions(MinecloneLib PUBLIC ASSETS_PATH=\"${Mineclone_SOURCE_DIR}/assets\")
endif()

if (MINECLONE_TESTING)
  target_compile_definitions(MinecloneLib PUBLIC MINECLONE_TESTING)
endif()

