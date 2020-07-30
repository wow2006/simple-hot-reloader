# ${CMAKE_SOURCE_DIR}/CMakeLists.txt
cmake_minimum_required(VERSION 3.9)
project(hotrelead CXX)

find_package(SDL2 REQUIRED)

add_subdirectory(lib)
add_subdirectory(app)

