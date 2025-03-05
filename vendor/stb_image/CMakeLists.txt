
add_library(
  stb_image

  ./stb_image.cpp
  ./stb_image.h
)

target_include_directories(stb_image PUBLIC ./)
target_compile_features(stb_image PUBLIC cxx_std_11)

