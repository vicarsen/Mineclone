cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(Mineclone)

add_executable(${PROJECT_NAME}
    ./src/main.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./include
)

