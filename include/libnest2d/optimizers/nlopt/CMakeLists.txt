add_library(nloptOptimizer INTERFACE)

find_package(NLopt 1.4 QUIET)

if(NOT NLopt_FOUND)
    message(STATUS  "NLopt not found so downloading "
                    "and automatic build is performed...")

    include(${PROJECT_SOURCE_DIR}/external/nlopt/DownloadNLopt.cmake)

    # Now the library is downloaded, configured, built and installed and the find
    # command should not have any problem to find it. 
    find_package(NLopt 1.4 REQUIRED)
endif()

target_link_libraries(nloptOptimizer INTERFACE Nlopt::nlopt)

list(APPEND LIBNEST2D_SRCFILES 
${CMAKE_CURRENT_SOURCE_DIR}/simplex.hpp
${CMAKE_CURRENT_SOURCE_DIR}/subplex.hpp
${CMAKE_CURRENT_SOURCE_DIR}/genetic.hpp
${CMAKE_CURRENT_SOURCE_DIR}/nlopt_boilerplate.hpp
)
