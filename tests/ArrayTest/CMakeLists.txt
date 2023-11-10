cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(ArrayTest VERSION 1.0
                  DESCRIPTION "Benchmarks and error checks for ::Utils::Array"
                  LANGUAGES CXX)

add_executable(${PROJECT_NAME} ./src/main.cpp)

# target_link_libraries(${PROJECT_NAME} PUBLIC MinecloneCore)
target_include_directories(${PROJECT_NAME} PUBLIC ../../engine/include)

target_link_libraries(${PROJECT_NAME} PUBLIC BenchmarkLib TestLib)

