# ${CMAKE_SOURCE_DIR}/app/CMakeLists.txt
add_executable(
  hot-reload-app
  main.cpp
  watchFile.cpp
  workingThread.cpp
)

target_link_libraries(
  hot-reload-app
  PUBLIC
  SDL2::SDL2
  fmt::fmt-header-only
  SDL2::Image
)

set_target_properties(
  hot-reload-app
  PROPERTIES
  CXX_EXTENSIONS        OFF
  CXX_STANDARD          17
  CXX_STANDARD_REQUIRED ON
)

