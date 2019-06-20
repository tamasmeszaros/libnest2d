cmake_minimum_required(VERSION 3.0)

project(Libnest2D)

# if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
#     # Update if necessary
#     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long ")
# endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)

# Add our own cmake module path.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules/)

option(LIBNEST2D_UNITTESTS "If enabled, googletest framework will be downloaded
    and the provided unit tests will be included in the build." OFF)

option(LIBNEST2D_BUILD_EXAMPLES "If enabled, examples will be built." OFF)

option(LIBNEST2D_HEADER_ONLY "If enabled static library will not be built." ON)

set(LIBNEST2D_DEP_DIR ${CMAKE_BINARY_DIR}/dependencies CACHE STRING "Dependencies location")

list(APPEND CMAKE_PREFIX_PATH ${LIBNEST2D_DEP_DIR})

set(GEOMETRY_BACKENDS clipper boost eigen)
set(LIBNEST2D_GEOMETRIES clipper CACHE STRING "Geometry backend")
set_property(CACHE LIBNEST2D_GEOMETRIES PROPERTY STRINGS ${GEOMETRY_BACKENDS})
list(FIND GEOMETRY_BACKENDS ${LIBNEST2D_GEOMETRIES} GEOMETRY_TYPE)
if(${GEOMETRY_TYPE} EQUAL -1)
    message(FATAL_ERROR "Option ${LIBNEST2D_GEOMETRIES} not supported, valid entries are ${GEOMETRY_BACKENDS}")
endif()

set(OPTIMIZERS nlopt optimlib)
set(LIBNEST2D_OPTIMIZER nlopt CACHE STRING "Optimization backend")
set_property(CACHE LIBNEST2D_OPTIMIZER PROPERTY STRINGS ${OPTIMIZERS})
list(FIND OPTIMIZERS ${LIBNEST2D_OPTIMIZER} OPTIMIZER_TYPE)
if(${OPTIMIZER_TYPE} EQUAL -1)
    message(FATAL_ERROR "Option ${LIBNEST2D_OPTIMIZER} not supported, valid entries are ${OPTIMIZERS}")
endif()

set(THREADING std tbb omp none)
set(LIBNEST2D_THREADING std CACHE STRING "Multithreading")
set_property(CACHE LIBNEST2D_THREADING PROPERTY STRINGS ${THREADING})
list(FIND THREADING ${LIBNEST2D_THREADING} THREADING_TYPE)
if(${THREADING_TYPE} EQUAL -1)
    message(FATAL_ERROR "Option ${LIBNEST2D_THREADING} not supported, valid entries are ${THREADING}")
endif()

add_library(libnest2d INTERFACE)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/include)

set(LIBNEST2D_SRCFILES
    ${SRC_DIR}/libnest2d/libnest2d.hpp         # Templates only
    ${SRC_DIR}/libnest2d/geometry_traits.hpp
    ${SRC_DIR}/libnest2d/geometry_traits_nfp.hpp
    ${SRC_DIR}/libnest2d/common.hpp
    ${SRC_DIR}/libnest2d/optimizer.hpp
    ${SRC_DIR}/libnest2d/utils/metaloop.hpp
    ${SRC_DIR}/libnest2d/utils/rotfinder.hpp
    ${SRC_DIR}/libnest2d/utils/rotcalipers.hpp
    ${SRC_DIR}/libnest2d/utils/bigint.hpp
    ${SRC_DIR}/libnest2d/utils/rational.hpp
    ${SRC_DIR}/libnest2d/placers/placer_boilerplate.hpp
    ${SRC_DIR}/libnest2d/placers/bottomleftplacer.hpp
    ${SRC_DIR}/libnest2d/placers/nfpplacer.hpp
    ${SRC_DIR}/libnest2d/selections/selection_boilerplate.hpp
    ${SRC_DIR}/libnest2d/selections/filler.hpp
    ${SRC_DIR}/libnest2d/selections/firstfit.hpp
    ${SRC_DIR}/libnest2d/selections/djd_heuristic.hpp
    )

if(NOT ${LIBNEST2D_THREADING} STREQUAL "none")
    include(threading_${LIBNEST2D_THREADING})
    target_link_libraries(libnest2d INTERFACE ${LIBNEST2D_THREADING}Threading)
    target_compile_definitions(libnest2d INTERFACE LIBNEST2D_THREADING_${LIBNEST2D_THREADING})
endif()

add_subdirectory(${SRC_DIR}/libnest2d/backends/${LIBNEST2D_GEOMETRIES})
target_link_libraries(libnest2d INTERFACE ${LIBNEST2D_GEOMETRIES}Backend)
target_compile_definitions(libnest2d INTERFACE LIBNEST2D_GEOMETRIES_${LIBNEST2D_GEOMETRIES})

add_subdirectory(${SRC_DIR}/libnest2d/optimizers/${LIBNEST2D_OPTIMIZER})
target_link_libraries(libnest2d INTERFACE ${LIBNEST2D_OPTIMIZER}Optimizer)
target_compile_definitions(libnest2d INTERFACE LIBNEST2D_OPTIMIZER_${LIBNEST2D_OPTIMIZER})

# target_sources(libnest2d INTERFACE ${LIBNEST2D_SRCFILES})
# target_include_directories(libnest2d INTERFACE ${SRC_DIR})

if(NOT LIBNEST2D_HEADER_ONLY)
    set(LIBNAME libnest2d_${LIBNEST2D_GEOMETRIES}_${LIBNEST2D_OPTIMIZER})
    add_library(${LIBNAME} ${PROJECT_SOURCE_DIR}/src/libnest2d.cpp)
    target_link_libraries(${LIBNAME} PUBLIC libnest2d)
    target_compile_definitions(${LIBNAME} PUBLIC LIBNEST2D_STATIC)
    target_sources(${LIBNAME} PRIVATE ${LIBNEST2D_SRCFILES})
endif()

if(LIBNEST2D_BUILD_EXAMPLES)
    add_subdirectory(${PROJECT_SOURCE_DIR}/examples)
endif()

if(LIBNEST2D_UNITTESTS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()
