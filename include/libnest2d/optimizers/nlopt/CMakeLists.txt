add_library(nloptOptimizer INTERFACE)

require_package(NLopt 1.4 REQUIRED)

target_link_libraries(nloptOptimizer INTERFACE Nlopt::nlopt)

list(APPEND LIBNEST2D_SRCFILES 
${CMAKE_CURRENT_SOURCE_DIR}/simplex.hpp
${CMAKE_CURRENT_SOURCE_DIR}/subplex.hpp
${CMAKE_CURRENT_SOURCE_DIR}/genetic.hpp
${CMAKE_CURRENT_SOURCE_DIR}/nlopt_boilerplate.hpp
)
