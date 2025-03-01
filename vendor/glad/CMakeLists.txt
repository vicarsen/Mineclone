
add_library(
  glad

  ./src/gl.c
  ./src/vulkan.c

  ./include/KHR/khrplatform.h

  ./include/glad/gl.h
  ./include/glad/vulkan.h
)

target_include_directories(glad PUBLIC ./include)
target_compile_features(glad PUBLIC cxx_std_11)

