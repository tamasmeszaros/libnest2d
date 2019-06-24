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
            -D "RP_INSTALL_PREFIX:PATH=${RP_ARGS_INSTALL_PATH}"
            -D "RP_PACKAGE:STRING=${RP_ARGS_PACKAGE}"
            -D "RP_${RP_ARGS_PACKAGE}_COMPONENTS=\"${RP_ARGS_COMPONENTS}\""
            -D "RP_${RP_ARGS_PACKAGE}_OPTIONAL_COMPONENTSS=\"${RP_ARGS_OPTIONAL_COMPONENTS}\"" 
            -D "RP_${RP_ARGS_PACKAGE}_VERSION=\"${RP_ARGS_VERSION}\"" 
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

macro(require_package)
    cmake_parse_arguments(RP_ARGS 
        "QUIET;REQUIRED" 
        "VERSION;PACKAGE" 
        "" ${ARGN})

    if(NOT RP_ARGS_PACKAGE)
        set(RP_ARGS_PACKAGE ${ARGV0})
    endif()

    if(NOT RP_ARGS_VERSION)
        set(RP_ARGS_VERSION ${ARGV1})
    endif()

    find_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} QUIET ${RP_ARGS_UNPARSED_ARGUMENTS})

    if(NOT ${RP_ARGS_PACKAGE}_FOUND AND NOT RP_DISABLE_DOWNLOADING)
        download_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} 
                         $<RP_ARGS_QUIET:QUIET>
                         ${RP_ARGS_UNPARSED_ARGUMENTS} )
    endif()

    find_package(${RP_ARGS_PACKAGE} ${RP_ARGS_VERSION} 
                 $<RP_ARGS_QUIET:QUIET> $<RP_ARGS_REQUIRED:REQUIRED> ${RP_ARGS_UNPARSED_ARGUMENTS})
endmacro()
