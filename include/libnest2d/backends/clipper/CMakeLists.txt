add_library(clipperBackend INTERFACE)

require_package(Clipper 6.1 REQUIRED)
target_link_libraries(clipperBackend INTERFACE Clipper::Clipper)

# Clipper backend is not enough on its own, it still need some functions
# from Boost geometry
require_package(Boost 1.58 REQUIRED)

if(TARGET Boost::boost)
    target_link_libraries(clipperBackend INTERFACE Boost::boost )
elseif(Boost_INCLUDE_DIRS_FOUND)
    target_include_directories(clipperBackend INTERFACE $<BUILD_INTERFACE:${Boost_INCLUDE_DIRS}> )
endif()

list(APPEND LIBNEST2D_SRCFILES 
include/libnest2d/backends/clipper/geometries.hpp
include/libnest2d/backends/clipper/clipper_polygon.hpp
)
set(LIBNEST2D_SRCFILES ${LIBNEST2D_SRCFILES} PARENT_SCOPE)

install(TARGETS clipperBackend EXPORT Libnest2DTargets INCLUDES DESTINATION include)
set(LIBNEST2D_PUBLIC_PACKAGES "${LIBNEST2D_PUBLIC_PACKAGES};Boost;Clipper" CACHE INTERNAL "")
