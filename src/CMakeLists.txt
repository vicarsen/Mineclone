
set(
  HEADER_LIST
  "../include/mineclonelib/log.h"
  "../include/mineclonelib/cvar.h"
)

add_library(
  mineclonelib

  log.cpp
  cvar.cpp

  ${HEADER_LIST}
)

target_include_directories(mineclonelib PUBLIC ../include)
target_link_libraries(mineclonelib PUBLIC spdlog)
target_compile_features(mineclonelib PUBLIC cxx_std_20)

source_group(
  TREE "${PROJECT_SOURCE_DIR}/include/mineclonelib"
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)

