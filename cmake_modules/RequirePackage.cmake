
# RP Package manager default install dir will be set globally
set(RP_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/dependencies CACHE STRING "Dependencies location")

option(RP_DISABLE_DOWNLOADING "Disable downloading of packages effectively falling back to find_package functionality" OFF)

# This variable is used to gather dependencies for export (find_dependency will be called for each package)
# A list of the requested packages from all require_package calls.
set(RP_USED_PACKAGES "" CACHE INTERNAL "")

if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "")
endif()

if (BUILD_SHARED_LIBS)
    set(RP_INSTALL_SUBDIR shared)
    set(RP_INSTALL_OTHER_SUBDIR static)
else()
    set(RP_INSTALL_SUBDIR static)
    set(RP_INSTALL_OTHER_SUBDIR shared)
endif()

list(REMOVE_ITEM CMAKE_PREFIX_PATH ${RP_INSTALL_PREFIX}/${RP_INSTALL_OTHER_SUBDIR})
file(REMOVE_RECURSE ${RP_INSTALL_PREFIX}/${RP_INSTALL_OTHER_SUBDIR})
list(APPEND CMAKE_PREFIX_PATH ${RP_INSTALL_PREFIX}/${RP_INSTALL_SUBDIR})

function(download_package)

    cmake_parse_arguments(RP_ARGS 
        "QUIET" 
        "VERSION;PACKAGE;REPOSITORY_PATH;INSTALL_PATH" 
        "COMPONENTS;OPTIONAL_COMPONENTS" ${ARGN})

    if(NOT RP_ARGS_REPOSITORY_PATH)
        if(RP_REPOSITORY_DIR)
            set(RP_ARGS_REPOSITORY_PATH ${RP_REPOSITORY_DIR})
        else()
            set(RP_ARGS_REPOSITORY_PATH ${PROJECT_SOURCE_DIR}/external)
        endif()
    endif()

    if(NOT RP_ARGS_INSTALL_PATH)
        if(RP_INSTALL_PREFIX)
            set(RP_ARGS_INSTALL_PATH ${RP_INSTALL_PREFIX})
        else()
            set(RP_ARGS_INSTALL_PATH ${CMAKE_BINARY_DIR}/rp_packages)
        endif()
    endif()

    if(NOT RP_ARGS_PACKAGE)
        set(RP_ARGS_PACKAGE ${ARGV0})
    endif()

    if(NOT RP_ARGS_VERSION)
        set(RP_ARGS_VERSION ${ARGV1})
    endif()

    set(RP_BUILD_PATH ${CMAKE_BINARY_DIR}/rp_packages_build)

    file(MAKE_DIRECTORY ${RP_BUILD_PATH})

    # Hide output if requested
    if (RP_ARGS_QUIET)
        set(OUTPUT_QUIET "OUTPUT_QUIET")
    else()
        unset(OUTPUT_QUIET)
        message(STATUS "Downloading/updating ${RP_ARGS_PACKAGE}")
    endif()

    execute_process(
        COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" 
            -D "CMAKE_MAKE_PROGRAM:FILE=${CMAKE_MAKE_PROGRAM}" 
            -D "RP_INSTALL_PREFIX:PATH=${RP_ARGS_INSTALL_PATH}/${RP_INSTALL_SUBDIR}"
            -D "RP_PACKAGE:STRING=${RP_ARGS_PACKAGE}"
            -D "RP_${RP_ARGS_PACKAGE}_COMPONENTS=\"${RP_ARGS_COMPONENTS}\""
            -D "RP_${RP_ARGS_PACKAGE}_OPTIONAL_COMPONENTS=\"${RP_ARGS_OPTIONAL_COMPONENTS}\"" 
            -D "RP_${RP_ARGS_PACKAGE}_VERSION=\"${RP_ARGS_VERSION}\"" 
            -D "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
            ${RP_ARGS_REPOSITORY_PATH}
        
        RESULT_VARIABLE CONFIG_STEP_RESULT
        #OUTPUT_VARIABLE CONFIG_STEP_OUTP
        #ERROR_VARIABLE  CONFIG_STEP_OUTP
        ${OUTPUT_QUIET}
        WORKING_DIRECTORY "${RP_BUILD_PATH}"
    )

    if(CONFIG_STEP_RESULT)
        # message(ERROR "${RP_ARGS_PACKAGE} output:\n ${CONFIG_STEP_OUTP}")    
        message(FATAL_ERROR "CMake step for ${RP_ARGS_PACKAGE} failed: ${CONFIG_STEP_RESULT}")
    endif()

    # Can proceed with the build step
    execute_process(
        COMMAND ${CMAKE_COMMAND} --build . --target rp_${RP_ARGS_PACKAGE}
        RESULT_VARIABLE BUILD_STEP_RESULT
        #OUTPUT_VARIABLE BUILD_STEP_OUTP
        #ERROR_VARIABLE  BUILD_STEP_OUTP
        ${OUTPUT_QUIET}
        WORKING_DIRECTORY "${RP_BUILD_PATH}"
    )

    if(BUILD_STEP_RESULT)
        # message(ERROR "${RP_ARGS_PACKAGE} output:\n ${BUILD_STEP_OUTP}")
        message(FATAL_ERROR "Build step for ${RP_ARGS_PACKAGE} failed: ${BUILD_STEP_RESULT}")
    endif()

endfunction()

macro(require_package RP_ARGS_PACKAGE RP_ARGS_VERSION)    
    set(options REQUIRED QUIET NO_EXPORT)
    set(oneValueArgs "")
    set(multiValueArgs "")
    cmake_parse_arguments(RP_ARGS 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT RP_ARGS_VERSION)
        set(RP_ARGS_VERSION ${ARGV1})
    endif()

    find_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} QUIET ${RP_ARGS_UNPARSED_ARGUMENTS})

    set(_REQUIRED "")
    if (RP_ARGS_REQUIRED) 
        set(_REQUIRED "REQUIRED")
    endif ()

    set(_QUIET "")
    if (RP_ARGS_QUIET) 
        set(_QUIET "QUIET")
    endif ()
    
    if(NOT ${RP_ARGS_PACKAGE}_FOUND )
        if (NOT RP_DISABLE_DOWNLOADING)
            download_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} 
                         ${_QUIET}
                         ${RP_ARGS_UNPARSED_ARGUMENTS} )
        endif()

        find_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} 
            ${_QUIET} ${_REQUIRED} ${RP_ARGS_UNPARSED_ARGUMENTS})
    endif()
    
    if (NOT RP_ARGS_NO_EXPORT)
        list(APPEND RP_USED_PACKAGES ${RP_ARGS_PACKAGE})
        set(RP_USED_PACKAGES "${RP_USED_PACKAGES}" CACHE INTERNAL "")
    endif()
    
endmacro()
