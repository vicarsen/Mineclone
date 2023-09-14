cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(./vendor/glad)
add_subdirectory(./vendor/glfw)
add_subdirectory(./vendor/glm)
add_subdirectory(./vendor/spdlog)

project(Mineclone)

add_executable(${PROJECT_NAME}
    ./src/main.cpp

    ./src/logger.cpp
    ./src/files.cpp
    ./src/mathematics.cpp
    ./src/stb_image.cpp

    ./src/input.cpp
    ./src/window.cpp
    ./src/render.cpp

    ./src/transform.cpp
    ./src/blocks.cpp
    ./src/world.cpp
    ./src/player.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./include
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Glad
    glfw
    glm::glm
    spdlog
)

