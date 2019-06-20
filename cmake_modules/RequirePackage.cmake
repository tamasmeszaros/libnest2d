function(require_package)

cmake_parse_arguments(RP_ARGS "QUIET" "PACKAGE;REPOSITORY_PATH;INSTALL_PATH" "" ${ARGN})

if(NOT RP_ARGS_REPOSITORY_PATH)
    set(RP_ARGS_REPOSITORY_PATH ${PROJECT_SOURCE_DIR}/external)
endif()

if(NOT RP_ARGS_INSTALL_PATH)
    set(RP_ARGS_INSTALL_PATH ${CMAKE_BINARY_DIR}/dependencies)
endif()

if(NOT RP_ARGS_PACKAGE)
    set(RP_ARGS_PACKAGE ${ARGV0})
endif()

set(DEP_BUILD_PATH ${CMAKE_BINARY_DIR}/dependencies-build)

file(MAKE_DIRECTORY ${DEP_BUILD_PATH})

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
        ${RP_ARGS_REPOSITORY_PATH}
    
    RESULT_VARIABLE CONFIG_STEP_RESULT
    #OUTPUT_VARIABLE CONFIG_STEP_OUTP
    #ERROR_VARIABLE  CONFIG_STEP_OUTP
    ${OUTPUT_QUIET}
    WORKING_DIRECTORY "${DEP_BUILD_PATH}"
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
    WORKING_DIRECTORY "${DEP_BUILD_PATH}"
)

if(BUILD_STEP_RESULT)
    # message(ERROR "${RP_ARGS_PACKAGE} output:\n ${BUILD_STEP_OUTP}")
    message(FATAL_ERROR "Build step for ${RP_ARGS_PACKAGE} failed: ${BUILD_STEP_RESULT}")
endif()

endfunction()