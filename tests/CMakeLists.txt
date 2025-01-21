
add_executable(windowtest window.cpp)

target_compile_features(windowtest PUBLIC cxx_std_20)
target_link_libraries(windowtest PUBLIC MinecloneLib)

add_test(NAME testwindowtest COMMAND windowtest)

add_executable(contexttest context.cpp)

target_compile_features(contexttest PUBLIC cxx_std_20)
target_link_libraries(contexttest PUBLIC MinecloneLib)

add_test(NAME testcontexttest COMMAND contexttest)

add_executable(inputtest input.cpp)

target_compile_features(inputtest PUBLIC cxx_std_20)
target_link_libraries(inputtest PUBLIC MinecloneLib)

add_test(NAME testinputtest COMMAND inputtest)

