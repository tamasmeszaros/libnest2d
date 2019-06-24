add_library(clipperBackend INTERFACE)
include(RequirePackage)

require_package(Clipper 6.1 REQUIRED)
target_link_libraries(clipperBackend INTERFACE Clipper::Clipper)

# Clipper backend is not enough on its own, it still needs some functions
# from Boost geometry
require_package(boost 1.58 REQUIRED)

if(TARGET Boost::boost)
    target_include_directories(clipperBackend INTERFACE Boost::boost )
elseif(Boost_INCLUDE_DIRS_FOUND)
    target_include_directories(clipperBackend INTERFACE ${Boost_INCLUDE_DIRS} )
endif()

