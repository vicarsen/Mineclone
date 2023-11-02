cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(Glad)

add_library(${PROJECT_NAME}
    ./src/gl.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./include
)

