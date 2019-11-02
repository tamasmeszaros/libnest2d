cmake_minimum_required(VERSION 3.1)

project(Deps)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake_modules)

include(RequirePackage)

require_package(Clipper REQUIRED)
