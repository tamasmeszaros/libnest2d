function(require_package)

cmake_parse_arguments(RP_ARGS "PROJ" "REPOSITORY_PATH" "" ${ARGN})

if(NOT RP_ARGS_REPOSITORY_PATH)
    set(RP_ARGS_REPOSITORY_PATH ${PROJECT_SOURCE_DIR}/external)
endif()

# configure_file()
execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" -D "CMAKE_MAKE_PROGRAM:FILE=${CMAKE_MAKE_PROGRAM}" .
    WORKING_DIRECTORY "${RP_ARGS_REPOSITORY_PATH}"
)

execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    #RESULT_VARIABLE result
    #OUTPUT_VARIABLE outp
    #${OUTPUT_QUIET}
    WORKING_DIRECTORY "${RP_ARGS_REPOSITORY_PATH}"
)

endfunction()

