cmake_minimum_required(VERSION 3.1)
project(RequiredPackages)

function(_message)
  message(${ARGV})
endfunction()

macro(message)
  if (NOT CMAKE_FIND_PACKAGE_NAME AND NOT ${CMAKE_FIND_PACKAGE_NAME}_FIND_QUIETLY)
    _message(${ARGV})
  endif ()
endmacro()

find_package(Git QUIET REQUIRED)

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

# If RP_PACKAGE is defined and non-empty, this subdir was called by RequirePackage module.
# Otherwise it was called by another means as a normal subdirectory or parent project

set(RP_ALL_TARGETS "")
set(RP_PACKAGE_SETUP_SCRIPT "" CACHE STRING "Specify custom package setup script.")
set(RP_PACKAGE_MOD_SCRIPT ""   CACHE STRING "Specify a script where package download targets can be tweaked.")
if (NOT AS_RP_PROCESS)

  set (RP_FORCE_DOWNLOADING OFF CACHE BOOL "Force all packages to be built even if installed in system.")

  # Instruction: append package names to RP_ALL_TARGETS to be included in ALL
  if (NOT RP_PACKAGE_SETUP_SCRIPT)
    include(PackageSetup.cmake)
  else ()
    include(${RP_PACKAGE_SETUP_SCRIPT})
  endif ()

endif()

if (RP_INSTALL_PREFIX)
    set(CMAKE_INSTALL_PREFIX "${RP_INSTALL_PREFIX}" CACHE STRING "" FORCE)
endif ()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build the packages as shared libraries.")
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_INSTALL_PREFIX})
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake-modules)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../cmake-modules/overrides)

if (NOT AS_RP_PROCESS)
  message(STATUS "Package summary:")
endif ()

foreach(p ${RP_PACKAGES})
  if (NOT RP_PACKAGE)
    set(RP_${p}_FOUND FALSE CACHE INTERNAL "")
  endif ()
  if (RP_FORCE_DOWNLOADING AND NOT AS_RP_PROCESS)
    list(FIND RP_ALL_TARGETS ${p} _idx)
    if (_idx LESS 0)
      message(STATUS "Available: ${p}")
    else ()
      message(STATUS "Selected:  ${p}")
    endif ()
  endif()
endforeach()

# Filter out packages that are present in system
if (NOT RP_FORCE_DOWNLOADING AND NOT AS_RP_PROCESS)
  foreach(p ${RP_PACKAGES})
    find_package(${p} ${RP_${p}_VERSION} QUIET COMPONENTS ${RP_${p}_COMPONENTS})
    if (${p}_FOUND)
      set (RP_${p}_FOUND TRUE CACHE INTERNAL "")
      message(STATUS   "Installed: ${p}")
      list(REMOVE_ITEM RP_ALL_TARGETS ${p})
    else ()
      list(FIND RP_ALL_TARGETS ${p} _idx)
      if (_idx LESS 0)
        message(STATUS "Available: ${p}")
      else()
        message(STATUS "Selected:  ${p}")
      endif ()
    endif ()
  endforeach()
endif ()

get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

function(require_dependency dep_name )

  cmake_parse_arguments(RD_ARGS "REQUIRED;QUIET" "VERSION" "COMPONENTS" ${ARGN})

  list(FIND RP_PACKAGES ${dep_name} _idx)
  set(_found_target true)
  if (_idx LESS 0)
    set(_found_target false)
  endif ()
  
  # set(dep_ver ${ARGV1})

  # TODO: Forward the highest version of RP and RD VERSION
  # TODO: Forward the union of components of RP and RD COMPONENTS and OPTIONAL COMPONENTS

  # Ignore REQUIRED, forward QUIET of RP

  if (AS_RP_PROCESS)
    list(FIND ${PACKAGE}_DEPENDS rp_${dep_name} _is_added_idx)
    if (_is_added_idx GREATER -1 OR RP_${dep_name}_FOUND OR ${dep_name}_FOUND)
      return()
    endif ()
  endif ()

  if(NOT RP_FORCE_DOWNLOADING OR NOT _found_target)
    find_package(${dep_name} ${dep_ver} QUIET ${RD_ARGS_UNPARSED_ARGUMENTS})
    if (${dep_name}_FOUND)
      set(RP_${dep_name}_FOUND TRUE CACHE INTERNAL "")
    endif()

    if(RP_${dep_name}_FOUND AND NOT RP_FIND_QUIETLY)
      message(STATUS "Link ${dep_name} (installed) to rp_${PACKAGE}")
    endif()
  endif()

  if(NOT RP_${dep_name}_FOUND)
    if (_found_target)
      list(APPEND ${PACKAGE}_DEPENDS rp_${dep_name})
      list(REMOVE_DUPLICATES ${PACKAGE}_DEPENDS)
      set(${PACKAGE}_DEPENDS "${${PACKAGE}_DEPENDS}" CACHE INTERNAL "")

      if(NOT RP_${dep_name}_FOUND AND NOT RP_FIND_QUIETLY)
        message(STATUS "Link rp_${dep_name} to rp_${PACKAGE}")
      endif()
    endif()
  endif()

endfunction()

include(ExternalProject)

function(rp_add_cmake_project projectname)

  cmake_parse_arguments(P_ARGS "" "INSTALL_DIR;BINARY_DIR" "CMAKE_ARGS" ${ARGN})

  set(_configs_line -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
  set(_postfix_line "")

  set(RP_RELEASE_POSTFIX "" CACHE STRING "")
  set(RP_DEBUG_POSTFIX "d" CACHE STRING "")
  set(RP_RELWITHDEBINFO_POSTFIX "_rwdi" CACHE STRING "")
  set(RP_MINSIZEREL_POSTFIX "_msr" CACHE STRING "")
  
  if (_is_multi)
    set(_configs ${CMAKE_CONFIGURATION_TYPES})
    foreach(_Conf ${_configs})
      string(TOUPPER "${_Conf}" _CONF)
      if (RP_${_CONF}_POSTFIX)
        list(APPEND _postfix_line -DCMAKE_${_CONF}_POSTFIX:STRING=${RP_${_CONF}_POSTFIX})
      endif ()
    endforeach()

    string(REPLACE ";" "|" _configs_ "${CMAKE_CONFIGURATION_TYPES}")
    set(_configs_line "-DCMAKE_CONFIGURATION_TYPES:STRING=${_configs_}")
  else ()
    set(_configs_line -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
      string(TOUPPER "${CMAKE_BUILD_TYPE}" _CONF)
      set(_conf ${RP_${_CONF}_POSTFIX})
      if(RP_${_CONF}_POSTFIX)
        list(APPEND _postfix_line -DCMAKE_${_CONF}_POSTFIX:STRING=${RP_${_CONF}_POSTFIX})
      endif ()
  endif ()
  
  string(REPLACE ";" "|" _module_paths "${CMAKE_MODULE_PATH}")

  ExternalProject_Add(
      rp_${projectname}
      INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}
      BINARY_DIR          ${RP_PACKAGE_BUILD_DIR}
      DOWNLOAD_EXTRACT_TIMESTAMP ON
      LIST_SEPARATOR |
      CMAKE_ARGS         
          -DCMAKE_INSTALL_PREFIX:STRING=${CMAKE_INSTALL_PREFIX}
          -DCMAKE_PREFIX_PATH:STRING=${CMAKE_INSTALL_PREFIX}
          -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
          -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
          -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
          # -DCMAKE_CONFIGURATION_TYPES:STRING=${}
          -DCMAKE_MODULE_PATH:STRING=${_module_paths}
          -DCMAKE_POSITION_INDEPENDENT_CODE:BOOL=ON
          "${_configs_line}"
          "${_postfix_line}"
          ${P_ARGS_CMAKE_ARGS}
     ${P_ARGS_UNPARSED_ARGUMENTS}
  )

endfunction()

if (APPLE)
    message("OS X SDK Path: ${CMAKE_OSX_SYSROOT}")
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        set(RP_OSX_TARGET "${CMAKE_OSX_DEPLOYMENT_TARGET}")
        message("OS X Deployment Target: ${RP_OSX_TARGET}")
    else ()
        # Attempt to infer the SDK version from the CMAKE_OSX_SYSROOT,
        # this is done because wxWidgets need the min version explicitly set
        string(REGEX MATCH "[0-9]+[.][0-9]+[.]sdk$" DEP_OSX_TARGET "${CMAKE_OSX_SYSROOT}")
        string(REGEX MATCH "^[0-9]+[.][0-9]+" DEP_OSX_TARGET "${DEP_OSX_TARGET}")

        if (NOT RP_OSX_TARGET)
            message(FATAL_ERROR "Could not determine OS X SDK version. Please use -DCMAKE_OSX_DEPLOYMENT_TARGET=<version>")
        endif ()

        message("OS X Deployment Target (inferred from SDK): ${RP_OSX_TARGET}")
    endif ()
    set (DEP_OSX_TARGET ${RP_OSX_TARGET})
endif ()

add_custom_target(rp ALL)

if (RP_PACKAGE)
  add_dependencies(rp rp_${RP_PACKAGE})
endif ()

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
  if(_is_multi)
    set(RP_PACKAGE_BUILD_DIR "rp_${PACKAGE}-build/")
  else()
    set(RP_PACKAGE_BUILD_DIR "rp_${PACKAGE}-build/${CMAKE_BUILD_TYPE}")
  endif()

  add_subdirectory("+${PACKAGE}" EXCLUDE_FROM_ALL)

endforeach()

foreach(PACKAGE IN ITEMS ${RP_PACKAGES})
  if(${PACKAGE}_DEPENDS)
    foreach(tgt ${${PACKAGE}_DEPENDS})
      if (NOT TARGET ${tgt})
        message(FATAL_ERROR "Required package target ${tgt} is not defined!")
      endif ()
    endforeach()
    add_dependencies(rp_${PACKAGE} ${${PACKAGE}_DEPENDS})
  endif()
endforeach()

if (NOT AS_RP_PROCESS)
  foreach(_pkg ${RP_ALL_TARGETS})
    add_dependencies(rp rp_${_pkg})
  endforeach()
endif()

if (RP_PACKAGE_MOD_SCRIPT)
  include(${RP_PACKAGE_MOD_SCRIPT})
endif ()

unset(RP_PACKAGE)
unset(RP_${RP_PACKAGE}_COMPONENTS)
unset(RP_${RP_PACKAGE}_COMPONENTS)
unset(RP_${RP_PACKAGE}_OPTIONAL_COMPONENTS)
unset(RP_${RP_PACKAGE}_VERSION)
unset(RP_FIND_REQUIRED)
unset(RP_FIND_QUIETLY)
unset(RP_PACKAGE_BUILD_DIR)
