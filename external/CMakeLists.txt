cmake_minimum_required(VERSION 3.0)

project(Dependencies)

set(RP_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dependencies CACHE STRING "Package install location")

macro(subdirlist result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      list(APPEND dirlist ${child})
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(RP_PACKAGES ${CMAKE_CURRENT_LIST_DIR})

message(STATUS "packages: ${RP_PACKAGES}")

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
    add_subdirectory(${PACKAGE} EXCLUDE_FROM_ALL)
endforeach()

get_directory_property(hasParent PARENT_DIRECTORY)

unset(DEPS_ALL)
if(NOT hasParent)
   set(DEPS_ALL "ALL")
endif()

add_custom_target(deps ${DEPS_ALL} DEPENDS ${RP_PACKAGES})
