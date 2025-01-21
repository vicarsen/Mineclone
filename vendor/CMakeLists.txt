
FetchContent_Declare(
  spdlog
  GIT_REPOSITORY https://github.com/gabime/spdlog
  GIT_TAG v1.15.0
)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 1.0.1
)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.4
)

FetchContent_MakeAvailable(spdlog glm glfw)

add_subdirectory(glad)
add_subdirectory(imgui)

