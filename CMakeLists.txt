cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(./vendor/glad)
add_subdirectory(./vendor/glfw)
add_subdirectory(./vendor/glm)
add_subdirectory(./vendor/spdlog)
add_subdirectory(./vendor/imgui)
add_subdirectory(./vendor/implot)

project(Mineclone)

add_executable(${PROJECT_NAME}
    ./src/main.cpp

    ./src/utils.cpp
    ./src/logger.cpp
    ./src/files.cpp
    ./src/mathematics.cpp
    ./src/aabb.cpp
    ./src/frustum.cpp

    ./src/input.cpp
    ./src/window.cpp
    ./src/render.cpp
    ./src/profiler.cpp

    ./src/gui.cpp

    ./src/stb_image.cpp
    ./src/transform.cpp
    ./src/blocks.cpp
    ./src/world.cpp
    ./src/player.cpp

    ./src/application.cpp

    ## ImGui Backends
    ./vendor/imgui/backends/imgui_impl_glfw.cpp
    ./vendor/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ./include

    ## ImGui Backends
    ./vendor/imgui/backends/
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    Glad
    glfw
    glm::glm
    spdlog
    ImGui
    ImPlot
)

