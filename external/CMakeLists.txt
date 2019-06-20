cmake_minimum_required(VERSION 3.0)

project(Dependencies)

set(RP_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dependencies CACHE STRING "Package install location")

file(GLOB RP_TARGETS LIST_DIRECTORIES true)

foreach(PACKAGE IN ITEMS clipper boost nlopt tbb)
    add_subdirectory(${PACKAGE} EXCLUDE_FROM_ALL)
    list(APPEND RP_TARGETS rp_${PACKAGE})
endforeach()

get_directory_property(hasParent PARENT_DIRECTORY)

unset(DEPS_ALL)
if(NOT hasParent)
   set(DEPS_ALL "ALL")
endif()

add_custom_target(deps ${DEPS_ALL} DEPENDS ${RP_TARGETS})
