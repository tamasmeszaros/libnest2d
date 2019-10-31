cmake_minimum_required(VERSION 3.1)

project(Libnest2D_example)

add_subdirectory(${PROJECT_SOURCE_DIR}/../ ${CMAKE_BINARY_DIR}/Libnest2D-build)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake_modules)
list(APPEND CMAKE_PREFIX_PATH ${RP_INSTALL_PREFIX})

find_package(Libnest2D REQUIRED)

add_executable(example ${PROJECT_SOURCE_DIR}/main.cpp
                       ${PROJECT_SOURCE_DIR}/../tools/svgtools.hpp
                       ${PROJECT_SOURCE_DIR}/../tools/printer_parts.cpp
                       ${PROJECT_SOURCE_DIR}/../tools/printer_parts.hpp
                       )

target_link_libraries(example libnest2d)
