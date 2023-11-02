cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(UtilsDemo)

add_executable(${PROJECT_NAME} PUBLIC ./src/main.cpp)

target_link_libraries(${PROJECT_NAME} PUBLIC MinecloneCore)

