# ${CMAKE_SOURCE_DIR}/app/CMakeLists.txt
download_file(
  "${CMAKE_CURRENT_LIST_DIR}/snakeHead.png"
  https://docs.google.com/uc?export=download&id=1bNg4q5rHEDN6d_Es83YGj_A6MLD3j_bK
  4f7e9f885048d65e569cf598e73605ba
)
file(
  COPY        "${CMAKE_CURRENT_LIST_DIR}/snakeHead.png"
  DESTINATION ${CMAKE_BINARY_DIR}
)

download_file(
  "${CMAKE_CURRENT_LIST_DIR}/apple.png"
  https://docs.google.com/uc?export=download&id=1i1bKxV1qC0MFR0tC-ubS2QMPLMcUReDL
  06288bbac2630aeff3c4a39f5433496c
)
file(
  COPY        "${CMAKE_CURRENT_LIST_DIR}/apple.png"
  DESTINATION ${CMAKE_BINARY_DIR}
)

add_executable(
  hot-reload-app
  main.cpp
  watchFile.cpp
  workingThread.cpp
)

target_link_libraries(
  hot-reload-app
  PUBLIC
  warning
  SDL2::SDL2
  SDL2::Image
  fmt::fmt-header-only
)

target_compile_definitions(
  hot-reload-app
  PUBLIC
  BASE_DIR="${CMAKE_SOURCE_DIR}"
)

set_target_properties(
  hot-reload-app
  PROPERTIES
  CXX_EXTENSIONS        OFF
  CXX_STANDARD          17
  CXX_STANDARD_REQUIRED ON
)

