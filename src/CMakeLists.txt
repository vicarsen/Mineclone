
add_library(
  MinecloneLib

  log.cpp
  ../include/log.h

  window.cpp
  ../include/window.h
)

target_include_directories(MinecloneLib PUBLIC ../include/)
target_link_libraries(MinecloneLib PUBLIC spdlog glm glfw glad)
target_compile_features(MinecloneLib PUBLIC cxx_std_20)

