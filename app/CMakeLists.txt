# ${CMAKE_SOURCE_DIR}/app/CMakeLists.txt
find_package(SDL2 REQUIRED)

add_executable(
  hot-reload-app
  main.cpp
)

target_include_directories(
  hot-reload-app
  PUBLIC
  ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(
  hot-reload-app
  PUBLIC
  ${SDL2_LIBRARIES}
)

set_target_properties(
  hot-reload-app
  PROPERTIES
  CXX_EXTENSIONS OFF
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

