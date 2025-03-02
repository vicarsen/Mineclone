
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.91.8-docking
)

FetchContent_MakeAvailable(imgui)

add_library(
  imgui

  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp

  ${imgui_SOURCE_DIR}/imconfig.h
  ${imgui_SOURCE_DIR}/imgui.h
  ${imgui_SOURCE_DIR}/imstb_rectpack.h
  ${imgui_SOURCE_DIR}/imstb_textedit.h
  ${imgui_SOURCE_DIR}/imstb_truetype.h
)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_compile_features(imgui PUBLIC cxx_std_20)

add_library(
  imgui_glfw

  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
)

target_compile_definitions(imgui_glfw PRIVATE GLFW_INCLUDE_NONE)
target_link_libraries(imgui_glfw PUBLIC glfw)
target_include_directories(imgui_glfw PUBLIC ${imgui_SOURCE_DIR})

add_library(
  imgui_opengl3

  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
)

target_link_libraries(imgui_opengl3 PUBLIC imgui glad)
target_include_directories(imgui_opengl3 PUBLIC ${imgui_SOURCE_DIR})

add_library(
  imgui_vulkan

  ./backends/custom/imgui_impl_vulkan.cpp
  ./backends/custom/imgui_impl_vulkan.h
  ./backends/custom/typedefs.h
)

target_link_libraries(imgui_vulkan PUBLIC imgui glad)
target_include_directories(imgui_vulkan PUBLIC ./)

