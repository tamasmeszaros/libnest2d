# RP Package manager default install dir will be set globally
set(RP_INSTALL_PREFIX ${PROJECT_BINARY_DIR}/dependencies CACHE STRING "Dependencies location")
set(RP_CONFIGURATION_TYPES ${CMAKE_CONFIGURATION_TYPES} CACHE STRING "Build configurations for the dependencies")
option(RP_ENABLE_DOWNLOADING "Enable downloading of bundled packages if not found in system." OFF)
include(CMakeDependentOption)
cmake_dependent_option(RP_FORCE_DOWNLOADING "Force downloading packages even if found." OFF "RP_ENABLE_DOWNLOADING" OFF)
set(RP_REPOSITORY_DIR ${PROJECT_SOURCE_DIR}/external CACHE STRING "Package repository location")

set(RP_BUILD_PATH ${PROJECT_BINARY_DIR}/rp_packages_build CACHE STRING "Binary dir for downloaded package builds")

mark_as_advanced(RP_BUILD_PATH)

# Packages for which require_package is called are gathered in this list.
set(RP_USED_PACKAGES "" CACHE INTERNAL "")

if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "")
endif()

list(APPEND CMAKE_PREFIX_PATH ${RP_INSTALL_PREFIX})

function(download_package)

    cmake_parse_arguments(RP_ARGS 
        "QUIET;REQUIRED" 
        "VERSION;PACKAGE;REPOSITORY_PATH;INSTALL_PATH" 
        "COMPONENTS;OPTIONAL_COMPONENTS" ${ARGN})

    if(NOT RP_ARGS_REPOSITORY_PATH)
        if(RP_REPOSITORY_DIR)
            set(RP_ARGS_REPOSITORY_PATH ${RP_REPOSITORY_DIR})
        else()
            set(RP_ARGS_REPOSITORY_PATH ${PROJECT_SOURCE_DIR}/external)
        endif()
    endif()

    set(_err_type "WARNING")
    if(RP_ARGS_REQUIRED)
        set(_err_type "FATAL_ERROR")
    endif()

    if(NOT EXISTS ${RP_ARGS_REPOSITORY_PATH}/+rp_${RP_ARGS_PACKAGE})
        if(NOT RP_ARGS_QUIET OR RP_ARGS_REQUIRED)
            message(${_err_type} "No package definition exists for ${RP_ARGS_PACKAGE}")
        endif()
    endif()

    if(NOT RP_ARGS_INSTALL_PATH)
        if(RP_INSTALL_PREFIX)
            set(RP_ARGS_INSTALL_PATH ${RP_INSTALL_PREFIX})
        else()
            set(RP_ARGS_INSTALL_PATH ${PROJECT_BINARY_DIR}/rp_packages)
        endif()
    endif()

    if(NOT RP_ARGS_PACKAGE)
        set(RP_ARGS_PACKAGE ${ARGV0})
    endif()

    if(NOT RP_ARGS_VERSION)
        set(RP_ARGS_VERSION ${ARGV1})
    endif()

    if (RP_ARGS_QUIET)
        set(OUTPUT_QUIET "OUTPUT_QUIET")
    else()
        unset(OUTPUT_QUIET)
    endif()

    if(NOT RP_ARGS_QUIET AND NOT EXISTS ${RP_BUILD_PATH})
        message(STATUS "\n--------------------------------------------------------------------------------\n"
                        "- Initializing RequirePackage repository cache\n"
                        "--------------------------------------------------------------------------------\n")
                    
        file(MAKE_DIRECTORY ${RP_BUILD_PATH})

        execute_process(
            COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -A "${CMAKE_GENERATOR_PLATFORM}" 
                -D "CMAKE_MAKE_PROGRAM:FILE=${CMAKE_MAKE_PROGRAM}"
                -D "CMAKE_INSTALL_PREFIX:PATH=${RP_ARGS_INSTALL_PATH}"
                -D "RP_FORCE_DOWNLOADING:BOOL=${RP_FORCE_DOWNLOADING}"
                -D "BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}"
                ${RP_ARGS_REPOSITORY_PATH}
            RESULT_VARIABLE CONFIG_STEP_RESULT
            ${OUTPUT_QUIET}
            WORKING_DIRECTORY "${RP_BUILD_PATH}"
        )

        if(CONFIG_STEP_RESULT)
            if(NOT RP_ARGS_QUIET OR RP_ARGS_REQUIRED)    
                message(${_err_type} "CMake step for ${RP_ARGS_PACKAGE} failed: ${CONFIG_STEP_RESULT}")
            endif()
        endif()

    endif()
    
    # Hide output if requested
    if (NOT RP_ARGS_QUIET)
        message(STATUS  "\n--------------------------------------------------------------------------------\n"
                          "- Downloading/updating ${RP_ARGS_PACKAGE}\n"
                          "--------------------------------------------------------------------------------\n")
    endif()

    set(_CONFIGS "${RP_CONFIGURATION_TYPES}")
    if (EXISTS ${RP_BUILD_PATH}/+${RP_ARGS_PACKAGE}/configs.txt)
        file(READ ${RP_BUILD_PATH}/+${RP_ARGS_PACKAGE}/configs.txt _CONFIGS)
        
        if(NOT RP_ARGS_QUIET)
            message(STATUS "${RP_ARGS_PACKAGE} configs: ${_CONFIGS}")
        endif()
    endif()

    if (NOT _CONFIGS) # No config is specified, default to release
        set(_CONFIGS "Release")
    endif()

    set(_configured FALSE)
    get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    
    foreach(_conf IN ITEMS ${_CONFIGS})
    
        if(NOT RP_ARGS_QUIET)
            message(STATUS "Building config: ${_conf} of package ${RP_ARGS_PACKAGE}\n")
        endif()

        if(NOT _configured OR NOT _is_multi)
            execute_process(
                COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -A "${CMAKE_GENERATOR_PLATFORM}" 
                    -D "CMAKE_MAKE_PROGRAM:FILE=${CMAKE_MAKE_PROGRAM}"
                    -D "CMAKE_BUILD_TYPE:STRING=${_conf}" 
                    -D "CMAKE_INSTALL_PREFIX:PATH=${RP_ARGS_INSTALL_PATH}"
                    -D "RP_PACKAGE:STRING=${RP_ARGS_PACKAGE}"
                    -D "RP_FORCE_DOWNLOADING:BOOL=${RP_FORCE_DOWNLOADING}"
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
                if(NOT RP_ARGS_QUIET OR RP_ARGS_REQUIRED)    
                    message(${_err_type} "CMake step for ${RP_ARGS_PACKAGE} failed: ${CONFIG_STEP_RESULT}")
                endif()
            else()
                set(_configured TRUE)
            endif()
        endif()

        # Can proceed with the build step
        execute_process(
            COMMAND ${CMAKE_COMMAND} --build . --target rp_${RP_ARGS_PACKAGE} --config ${_conf} --clean-first
            RESULT_VARIABLE BUILD_STEP_RESULT
            #OUTPUT_VARIABLE BUILD_STEP_OUTP
            ERROR_VARIABLE  BUILD_STEP_OUTP
            ${OUTPUT_QUIET}
            WORKING_DIRECTORY "${RP_BUILD_PATH}"
        )

        if(BUILD_STEP_RESULT)
            if(NOT RP_ARGS_QUIET OR RP_ARGS_REQUIRED)
                message(${_err_type} "Build step (${_conf}) for ${RP_ARGS_PACKAGE} failed: ${BUILD_STEP_OUTP}")
            endif()if(NOT RP_ARGS_QUIET OR RP_ARGS_REQUIRED)
            message(${_err_type} "Build step (${_conf}) for ${RP_ARGS_PACKAGE} failed: ${BUILD_STEP_OUTP}")
        endif()
        endif()
    endforeach(_conf IN ${_CONFIGS})

endfunction()

macro(require_package RP_ARGS_PACKAGE)    
    set(options REQUIRED QUIET)
    set(oneValueArgs "VERSION")
    set(multiValueArgs "")
    cmake_parse_arguments(RP_ARGS 
        "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # if(NOT RP_ARGS_VERSION)
    #     set(RP_ARGS_VERSION ${ARGV1})
    # endif()

    if(NOT RP_FORCE_DOWNLOADING)
        find_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} QUIET ${RP_ARGS_UNPARSED_ARGUMENTS})
    endif()

    set(_REQUIRED "")
    if (RP_ARGS_REQUIRED) 
        set(_REQUIRED "REQUIRED")
    endif ()

    set(_QUIET "")
    if (RP_ARGS_QUIET) 
        set(_QUIET "QUIET")
    endif ()
    
    if(NOT ${RP_ARGS_PACKAGE}_FOUND)
        if (RP_ENABLE_DOWNLOADING)
            download_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} ${_QUIET} ${RP_ARGS_UNPARSED_ARGUMENTS} )
        endif()

        find_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} 
            ${_QUIET} ${_REQUIRED} ${RP_ARGS_UNPARSED_ARGUMENTS})
    endif()
    
    list(APPEND RP_USED_PACKAGES ${RP_ARGS_PACKAGE})
    set(RP_USED_PACKAGES "${RP_USED_PACKAGES}" CACHE INTERNAL "")
    set(RP_${RP_ARGS_PACKAGE}_VERSION ${RP_ARGS_VERSION} CACHE INTERNAL "")
    set(ignoreMe "RP_${RP_ARGS_PACKAGE}_VERSION")
    
endmacro()

function(rp_install_versions_file dest)
    set(_out "")
    set(_fname ${RP_BUILD_PATH}/RPPackageVersions.cmake)
    foreach(p ${RP_USED_PACKAGES})
        set(_out "${_out}set(${p}_VERSION ${RP_${p}_VERSION})\n")
    endforeach()
    file(WRITE ${_fname} ${_out})
    install(FILES ${_fname} DESTINATION ${dest})
endfunction()
