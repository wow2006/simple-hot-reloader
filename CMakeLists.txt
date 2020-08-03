# ${CMAKE_SOURCE_DIR}/CMakeLists.txt
cmake_minimum_required(VERSION 3.9)
project(hotrelead CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(download_file)
include(add_warning)

find_package(fmt  REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)

