cmake_minimum_required(VERSION 3.10)

add_subdirectory(./vendor/googlebenchmark)

project(Tests)

add_library(BenchmarkLib INTERFACE)
target_link_libraries(BenchmarkLib INTERFACE benchmark::benchmark)

add_library(TestLib INTERFACE)

add_subdirectory(./ArrayTest)

