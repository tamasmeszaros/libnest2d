cmake_minimum_required(VERSION 3.1)

project(Libnest2D_example)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)

message(STATUS "PREFIX PATH = ${CMAKE_PREFIX_PATH}")
find_package(Libnest2D QUIET)

if(NOT Libnest2D_FOUND)
    message(STATUS "No installed version of Libnest2D has been found in the system. Falling back to subproject mode...")
    add_subdirectory(${PROJECT_SOURCE_DIR}/../ ${CMAKE_BINARY_DIR}/Libnest2D-build)
    add_library(Libnest2D::libnest2d ALIAS libnest2d)
else()
    message(STATUS "Libnest2D was found in ${Libnest2D_DIR}")
endif()

add_executable(example ${PROJECT_SOURCE_DIR}/main.cpp
                       ${PROJECT_SOURCE_DIR}/../tools/svgtools.hpp
                       ${PROJECT_SOURCE_DIR}/../tools/printer_parts.cpp
                       ${PROJECT_SOURCE_DIR}/../tools/printer_parts.hpp
                       )

target_link_libraries(example Libnest2D::libnest2d)
