cmake_minimum_required(VERSION 2.8)

project(Binpack2D)

enable_testing()

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Update if necessary
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long ")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)

# Add our own cmake module path.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules/)

option(BINPACK2D_UNITTESTS "If enabled, googletest framework will be downloaded
    and the provided unit tests will be included in the build." OFF)

#set(BINPACK2D_GEOMETRIES_TARGET "" CACHE STRING
#    "Build binpack2d with geometry classes implemented by the chosen target.")

#set(BINPACK2D_TEST_LIBRARIES "" CACHE STRING
#    "Libraries needed to compile the test executable for binpack2d.")


set(BINPACK2D_SRCFILES binpack2d/binpack2d.cpp
    binpack2d/binpack2d.hpp       # Templates only
    binpack2d.h         # Exports ready made types using template arguments
    binpack2d/geometry_traits.hpp
    binpack2d/geometries_io.hpp
    binpack2d/common.hpp
    binpack2d/placers/placer_boilerplate.hpp
    binpack2d/placers/bottomleftplacer.hpp
    binpack2d/placers/nfpplacer.hpp
    binpack2d/placers/geometries_nofitpolygon.hpp
    binpack2d/selections/selection_boilerplate.hpp
    binpack2d/selections/filler.hpp
    binpack2d/selections/firstfit.hpp
    binpack2d/selections/djd_heuristic.hpp
    )

if((NOT BINPACK2D_GEOMETRIES_TARGET) OR (BINPACK2D_GEOMETRIES_TARGET STREQUAL ""))
    message(STATUS "Binpack2D backend is default")

    if(NOT Boost_INCLUDE_DIRS_FOUND)
        find_package(Boost REQUIRED)
        # TODO automatic download of boost geometry headers
    endif()

    add_subdirectory(binpack2d/clipper_backend)

    set(BINPACK2D_GEOMETRIES_TARGET ${CLIPPER_LIBRARIES})

    include_directories(BEFORE ${CLIPPER_INCLUDE_DIRS})
    include_directories(${Boost_INCLUDE_DIRS})

    list(APPEND BINPACK2D_SRCFILES binpack2d/clipper_backend/clipper_backend.cpp
                                   binpack2d/clipper_backend/clipper_backend.hpp
                                   binpack2d/boost_alg.hpp)

else()
    message(STATUS "Binpack2D backend target is: ${BINPACK2D_GEOMETRIES_TARGET}")
endif()

add_library(binpack2d STATIC ${BINPACK2D_SRCFILES} )
target_link_libraries(binpack2d ${BINPACK2D_GEOMETRIES_TARGET})
target_include_directories(binpack2d PUBLIC ${CMAKE_SOURCE_DIR})

if(BINPACK2D_UNITTESTS)
    add_subdirectory(tests)
endif()

get_directory_property(hasParent PARENT_DIRECTORY)
if(hasParent)
    set(BINPACK2D_HEADERS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
endif()


