
add_executable(windowtest window.cpp)

target_compile_features(windowtest PUBLIC cxx_std_20)
target_link_libraries(windowtest PUBLIC MinecloneLib)

add_test(NAME testwindowtest COMMAND windowtest)

