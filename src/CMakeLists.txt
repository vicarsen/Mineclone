
add_library(
  MinecloneLib

  mineclone.cpp
  ../include/mineclone.h
)

target_include_directories(MinecloneLib PUBLIC ../include/)
target_link_libraries(MinecloneLib PUBLIC spdlog glm glfw glad)
target_compile_definitions(MinecloneLib PUBLIC cxx_std_20)

