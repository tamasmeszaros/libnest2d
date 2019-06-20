cmake_minimum_required(VERSION 3.0)

project(libnest2d_deps)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR}/dependencies CACHE STRING "Dependencies location")

add_subdirectory(boost EXCLUDE_FROM_ALL)
add_subdirectory(clipper EXCLUDE_FROM_ALL)
