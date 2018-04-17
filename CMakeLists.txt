cmake_minimum_required(VERSION 2.8)

project(Binpack2D)

# Add our own cmake module path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/)

option(BINPACK2D_UNITTESTS "If enabled, googletest framework will be downloaded
    and the provided unit tests will be included in the build." OFF)

#set(BINPACK2D_GEOMETRIES_TARGET "" CACHE STRING
#    "Build binpack2d with geometry classes implemented by the chosen target.")

set(BINPACK2D_TEST_LIBRARIES "" CACHE STRING
    "Libraries needed to compile the test executable for binpack2d.")


set(BINPACK2D_SRCFILES binpack2d.cpp
    binpack2d.hpp       # Templates only
    binpack2d.h         # Exports ready made types using template arguments
    geometries.hpp
    geometries.hpp
    geometry_traits.hpp
    geometries_io.hpp
    config.hpp
    )

if((NOT BINPACK2D_GEOMETRIES_TARGET) OR (BINPACK2D_GEOMETRIES_TARGET STREQUAL ""))
    message(STATUS "Binpack2D backend is default")

    if(NOT Boost_INCLUDE_DIRS_FOUND)
        find_package(Boost REQUIRED)
    endif()

    add_subdirectory(clipper_backend)

    set(BINPACK2D_GEOMETRIES_TARGET ${CLIPPER_LIBRARIES})

    include_directories(BEFORE ${CLIPPER_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})

    list(APPEND BINPACK2D_SRCFILES clipper_backend/clipper_backend.cpp
                                   clipper_backend/clipper_backend.hpp
                                   boost_alg.hpp)

else()
    message(STATUS "Binpack2D backend target is: ${BINPACK2D_GEOMETRIES_TARGET}")
endif()

add_library(binpack2d STATIC ${BINPACK2D_SRCFILES} )
target_link_libraries(binpack2d ${BINPACK2D_GEOMETRIES_TARGET})

if(BINPACK2D_UNITTESTS)
    add_subdirectory(tests)
endif()

set(BINPACK2D_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
