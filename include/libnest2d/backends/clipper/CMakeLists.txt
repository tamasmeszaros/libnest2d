add_library(clipperBackend INTERFACE)
include(RequirePackage)

if(TARGET clipper)
    target_link_libraries(clipperBackend INTERFACE clipper)
else()
    find_package(Clipper 6.1 QUIET)

    if(NOT CLIPPER_FOUND)

       require_package(Clipper INSTALL_PATH ${LIBNEST2D_DEP_DIR})

       find_package(Clipper 6.1 REQUIRED)
       
    endif()
    target_link_libraries(clipperBackend INTERFACE Clipper::Clipper)
endif()

find_package(Boost 1.58 QUIET)

# Clipper backend is not enough on its own, it still needs some functions
# from Boost geometry
if(NOT Boost_FOUND)
    require_package(boost INSTALL_PATH ${LIBNEST2D_DEP_DIR})
    find_package(Boost 1.58 REQUIRED)
endif()

if(TARGET Boost::boost)
    target_include_directories(clipperBackend INTERFACE Boost::boost )
elseif(Boost_INCLUDE_DIRS_FOUND)
    target_include_directories(clipperBackend INTERFACE ${Boost_INCLUDE_DIRS} )
endif()

