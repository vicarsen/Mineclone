cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JEMALLOC jemalloc)

pkg_search_module(JEMALLOC REQUIRED jemalloc)

add_subdirectory(./vendor/glad)
add_subdirectory(./vendor/glfw)
add_subdirectory(./vendor/glm)
add_subdirectory(./vendor/spdlog)
add_subdirectory(./vendor/imgui)
add_subdirectory(./vendor/implot)

project(MinecloneCore)

set(MinecloneCoreSources
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
)

set(MinecloneCoreIncludeDirs ./include ${JEMALLOC_INCLUDE_DIRS})

set(MinecloneCoreImGUIBackendSources
    ./vendor/imgui/backends/imgui_impl_glfw.cpp 
    ./vendor/imgui/backends/imgui_impl_opengl3.cpp)

set(MinecloneCoreImGUIBackendIncludeDirs ./vendor/imgui/backends/)

set(MinecloneCoreLibraries ${JEMALLOC_LIBRARIES} Glad glfw glm::glm spdlog ImGui ImPlot)

add_library(${PROJECT_NAME}
    ${MinecloneCoreSources}
    ${MinecloneCoreImGUIBackendSources}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${MinecloneCoreIncludeDirs}
    ${MinecloneCoreImGUIBackendIncludeDirs}
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    ${MinecloneCoreLibraries}
)

