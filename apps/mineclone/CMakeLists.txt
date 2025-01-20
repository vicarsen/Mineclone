
add_executable(
  Mineclone

  main.cpp
)

target_link_libraries(Mineclone PUBLIC MinecloneLib)
target_compile_features(Mineclone PUBLIC cxx_std_20)

