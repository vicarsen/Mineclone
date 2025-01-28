cmake_minimum_required(VERSION 3.27)

project(
  Mineclone
  VERSION 0.1
  DESCRIPTION "A voxel game/game engine"
  LANGUAGES C CXX
)

set(MINECLONE_TESTING ON CACHE BOOL "Whether or not unit tests will be run. This makes logging macros work in Release mode as well.")
set(MINECLONE_PROFILE ON CACHE BOOL "Whether or not to register profiling data.")
set(MINECLONE_DEBUG ON CACHE BOOL "Whether or not in debug mode.")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  include(CTest)

  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

include(FetchContent)

add_subdirectory(vendor)
add_subdirectory(src)
add_subdirectory(apps)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MINECLONE_BUILD_TESTING) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

