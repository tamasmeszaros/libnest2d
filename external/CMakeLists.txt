cmake_minimum_required(VERSION 3.1)

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

get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

function(require_dependency dep_name )

  cmake_parse_arguments(RD_ARGS "REQUIRED;QUIET" "VERSION" "COMPONENTS" ${ARGN})
  
  # set(dep_ver ${ARGV1})

  set(_QUIET "")
  if (RP_ARGS_QUIET) 
    set(_QUIET "QUIET")
  endif ()

  # TODO: Forward the highest version of RP and RD VERSION
  # TODO: Forward the union of components of RP and RD COMPONENTS and OPTIONAL COMPONENTS

  # Ignore REQUIRED

  if(NOT RP_FORCE_DOWNLOADING)
    find_package(${dep_name} ${dep_ver} ${_QUIET} ${RD_ARGS_UNPARSED_ARGUMENTS})
  endif()

  if(NOT ${dep_name}_FOUND)
    list(APPEND ${PACKAGE}_DEPENDS rp_${dep_name})
    list(REMOVE_DUPLICATES ${PACKAGE}_DEPENDS)
    set(${PACKAGE}_DEPENDS "${${PACKAGE}_DEPENDS}" CACHE INTERNAL "")
  endif()
endfunction()

# If RP_PACKAGE is defined and non-empty, this subdir was called by RequirePackage module.
# Otherwise it was called by another means as a normal subdirectory or parent project

if (NOT RP_PACKAGE)
  add_custom_target(rp ALL)
endif()

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
  if(_is_multi)
    set(RP_PACKAGE_BUILD_DIR "rp_${PACKAGE}-build/")
  else()
    set(RP_PACKAGE_BUILD_DIR "rp_${PACKAGE}-build/${CMAKE_BUILD_TYPE}")
  endif()

  add_subdirectory("+${PACKAGE}" EXCLUDE_FROM_ALL)

  if (NOT RP_PACKAGE)
    add_dependencies(rp rp_${PACKAGE})
  endif()
endforeach()

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
  if(${PACKAGE}_DEPENDS)
    add_dependencies(rp_${PACKAGE} ${${PACKAGE}_DEPENDS})
  endif()
endforeach()