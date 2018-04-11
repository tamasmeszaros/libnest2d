cmake_minimum_required(VERSION 2.8)

project(Binpack2D)

# Add our own cmake module path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules/)

option(BINPACK2D_UNITTESTS "If enabled, googletest framework will be downloaded
    and the provided unit tests will be included in the build." OFF)

#set(BINPACK2D_GEOMETRIES_TARGET "" CACHE STRING
#    "Build binpack2d with geometry classes implemented by the chosen target.")

set(BINPACK2D_TEST_LIBRARIES "" CACHE STRING
    "Libraries needed to compile the test executable for binpack2d.")

add_subdirectory(libnfp)

set(BINPACK2D_SRCFILES binpack2d.cpp
    binpack2d.hpp
    geometries.hpp
    geometries.hpp
    geometry_traits.hpp
    geometries_io.hpp
    ${LIBNFP_SOURCES}
    )


include_directories(${LIBNFP_INCLUDES})
add_library(binpack2d STATIC ${BINPACK2D_SRCFILES} ${LIBNFP_LIBRARIES})

if(BINPACK2D_GEOMETRIES_TARGET STREQUAL "")
    message(STATUS "Binpack2D backend target is default")
    list(APPEND BINPACK2D_SRCFILES default_backend.cpp)
else()
    message(STATUS "Binpack2D backend target is: ${BINPACK2D_GEOMETRIES_TARGET}")
    target_link_libraries(binpack2d ${BINPACK2D_GEOMETRIES_TARGET})
endif()

if(BINPACK2D_UNITTESTS)
    add_subdirectory(tests)
endif()

set(BINPACK2D_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
