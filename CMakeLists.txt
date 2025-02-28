cmake_minimum_required(VERSION 3.15..3.31)

project(
  Mineclone
  VERSION 0.1.0
  DESCRIPTION "A Minecraft clone written over the course of university"
  LANGUAGES CXX
)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_EXTENSIONS OFF)
  
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  include(CTest)

  find_package(Doxygen)
  if (Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()
endif()

include(FetchContent)

add_subdirectory(vendor)
add_subdirectory(assets)
add_subdirectory(src)
add_subdirectory(apps)

if ((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MINECLONE_BUILD_TESTING) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()

