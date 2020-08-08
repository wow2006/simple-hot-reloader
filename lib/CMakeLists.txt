# ${CMAKE_SOURCE_DIR}/lib/CMakeLists.txt
add_library(
  hot-reload-lib
  MODULE
  library.cpp
)

target_include_directories(
  hot-reload-lib
  PRIVATE
  ${RAPIDJSON_INCLUDE_DIRS}
)

target_link_libraries(
  hot-reload-lib
  PRIVATE
  warning
  SDL2::SDL2
  SDL2::Image
  fmt::fmt-header-only
  SDL2_ttf
)

set_target_properties(
  hot-reload-lib
  PROPERTIES
  CXX_EXTENSIONS OFF
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

