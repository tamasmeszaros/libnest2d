cmake_minimum_required(VERSION 3.0)

project(RequiredPackages)

option(RP_DISABLE_DOWNLOADING "Disable downloading of packages effectively falling back to find_package functionality" OFF)
set(RP_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/rp_packages CACHE STRING "Package install location")
set(RP_REPOSITORY_DIR ${PROJECT_SOURCE_DIR}/external CACHE STRING "Package repository location")
set(RP_DEBUG_POSTFIX "d" CACHE STRING "Preferred postfix for libraries built in Debug mode")

macro(subdirlist result curdir)
  file(GLOB children RELATIVE ${curdir} ${curdir}/*)
  set(dirlist "")
  foreach(child ${children})
    if(IS_DIRECTORY ${curdir}/${child})
      string(REGEX MATCH "^\\+([a-zA-Z0-9]+)" is_package ${child})
      if(is_package) 
         list(APPEND dirlist ${CMAKE_MATCH_1})
      endif()
    endif()
  endforeach()
  set(${result} ${dirlist})
endmacro()

subdirlist(RP_PACKAGES ${CMAKE_CURRENT_LIST_DIR})

get_directory_property(hasParent PARENT_DIRECTORY)

unset(DEPS_ALL)
if(NOT hasParent)
   set(DEPS_ALL "ALL")
endif()

add_custom_target(rp ${DEPS_ALL})

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
    option(RP_ENABLE_${PACKAGE} "Enable package ${PACKAGE}" ON)
    add_subdirectory("+${PACKAGE}" EXCLUDE_FROM_ALL)
    add_dependencies(rp rp_${PACKAGE})
endforeach()
