cmake_minimum_required(VERSION 3.0)

project(RequiredPackages)

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

if (NOT RP_PACKAGES)
    subdirlist(RP_PACKAGES ${CMAKE_CURRENT_LIST_DIR})
endif()

get_directory_property(hasParent PARENT_DIRECTORY)

unset(DEPS_ALL)
if(NOT hasParent)
   set(DEPS_ALL "ALL")
endif()

add_custom_target(rp ${DEPS_ALL})

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
    add_subdirectory("+${PACKAGE}" EXCLUDE_FROM_ALL)
    add_dependencies(rp rp_${PACKAGE})
endforeach()
