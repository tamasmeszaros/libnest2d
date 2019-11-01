add_library(nloptOptimizer INTERFACE)

require_package(NLopt 1.4 REQUIRED)

target_link_libraries(nloptOptimizer INTERFACE NLopt::nlopt)

list(APPEND LIBNEST2D_SRCFILES 
include/libnest2d/optimizers/nlopt/simplex.hpp
include/libnest2d/optimizers/nlopt/subplex.hpp
include/libnest2d/optimizers/nlopt/genetic.hpp
include/libnest2d/optimizers/nlopt/nlopt_boilerplate.hpp
)
set(LIBNEST2D_SRCFILES ${LIBNEST2D_SRCFILES} PARENT_SCOPE)

install(TARGETS nloptOptimizer EXPORT Libnest2DTargets INCLUDES DESTINATION include)
set(LIBNEST2D_PUBLIC_PACKAGES "${LIBNEST2D_PUBLIC_PACKAGES};NLopt" CACHE INTERNAL "")

