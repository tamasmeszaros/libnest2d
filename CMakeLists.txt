cmake_minimum_required(VERSION 2.8)

project(Libnest2D)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    # Update if necessary
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long ")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED)

# Add our own cmake module path.
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules/)

option(LIBNEST2D_UNITTESTS "If enabled, googletest framework will be downloaded
    and the provided unit tests will be included in the build." OFF)

option(LIBNEST2D_BUILD_EXAMPLES "If enabled, examples will be built." OFF)

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
    ${SRC_DIR}/libnest2d/placers/placer_boilerplate.hpp
    ${SRC_DIR}/libnest2d/placers/bottomleftplacer.hpp
    ${SRC_DIR}/libnest2d/placers/nfpplacer.hpp
    ${SRC_DIR}/libnest2d/selections/selection_boilerplate.hpp
    ${SRC_DIR}/libnest2d/selections/filler.hpp
    ${SRC_DIR}/libnest2d/selections/firstfit.hpp
    ${SRC_DIR}/libnest2d/selections/djd_heuristic.hpp
    )

set(TBB_STATIC ON)
find_package(TBB QUIET)
if(TBB_FOUND)
    message(STATUS "Parallelization with Intel TBB")
    target_include_directories(libnest2d INTERFACE ${TBB_INCLUDE_DIRS})
    target_compile_definitions(libnest2d INTERFACE ${TBB_DEFINITIONS} -DUSE_TBB)
    if(MSVC)
       # Suppress implicit linking of the TBB libraries by the Visual Studio compiler.
       target_compile_definitions(libnest2d INTERFACE -D__TBB_NO_IMPLICIT_LINKAGE)
    endif()
    # The Intel TBB library will use the std::exception_ptr feature of C++11.
    target_compile_definitions(libnest2d INTERFACE -DTBB_USE_CAPTURED_EXCEPTION=1)

    target_link_libraries(libnest2d INTERFACE ${TBB_LIBRARIES})
else()
   find_package(OpenMP QUIET)
   if(OpenMP_CXX_FOUND)
       message(STATUS "Parallelization with OpenMP")
       target_include_directories(libnest2d INTERFACE OpenMP::OpenMP_CXX)
       target_link_libraries(libnest2d INTERFACE OpenMP::OpenMP_CXX)
   endif()
endif()

add_subdirectory(${SRC_DIR}/libnest2d/backends/clipper)
add_subdirectory(${SRC_DIR}/libnest2d/optimizers/nlopt)

target_sources(libnest2d INTERFACE ${LIBNEST2D_SRCFILES})
target_include_directories(libnest2d INTERFACE ${SRC_DIR})

if(LIBNEST2D_BUILD_EXAMPLES)
    add_executable(example     examples/main.cpp
    #                           tools/libnfpglue.hpp
    #                           tools/libnfpglue.cpp
                               tools/nfp_svgnest.hpp
                               tools/nfp_svgnest_glue.hpp
                               tools/svgtools.hpp
                               tests/printer_parts.cpp
                               tests/printer_parts.h
                               )
    target_link_libraries(example libnest2d NloptOptimizer ClipperBackend)

#    add_executable(example examples/main.cpp
##                           tools/libnfpglue.hpp
##                           tools/libnfpglue.cpp
#                           tools/nfp_svgnest.hpp
#                           tools/nfp_svgnest_glue.hpp
#                           tools/svgtools.hpp
#                           tests/printer_parts.cpp
#                           tests/printer_parts.h
#                           ${LIBNEST2D_SRCFILES}
#                           )
#    set(TBB_STATIC ON)
#    find_package(TBB QUIET)
#    if(TBB_FOUND)
#        message(STATUS "Parallelization with Intel TBB")
#        target_include_directories(example PUBLIC ${TBB_INCLUDE_DIRS})
#        target_compile_definitions(example PUBLIC ${TBB_DEFINITIONS} -DUSE_TBB)
#        if(MSVC)
#           # Suppress implicit linking of the TBB libraries by the Visual Studio compiler.
#           target_compile_definitions(example PUBLIC -D__TBB_NO_IMPLICIT_LINKAGE)
#        endif()
#        # The Intel TBB library will use the std::exception_ptr feature of C++11.
#        target_compile_definitions(example PUBLIC -DTBB_USE_CAPTURED_EXCEPTION=1)

#        target_link_libraries(example ${TBB_LIBRARIES})
#   else()
#       find_package(OpenMP QUIET)
#       if(OpenMP_CXX_FOUND)
#           message(STATUS "Parallelization with OpenMP")
#           target_include_directories(example PUBLIC OpenMP::OpenMP_CXX)
#           target_link_libraries(example OpenMP::OpenMP_CXX)
#       endif()
# endif()

#    target_link_libraries(example ${LIBNEST2D_LIBRARIES})
#    target_include_directories(example PUBLIC ${LIBNEST2D_HEADERS})
endif()

if(LIBNEST2D_UNITTESTS)
    add_subdirectory(${PROJECT_SOURCE_DIR}/tests)
endif()

#get_directory_property(hasParent PARENT_DIRECTORY)
#if(hasParent)
#    set(LIBNEST2D_INCLUDES ${LIBNEST2D_HEADERS} PARENT_SCOPE)
#    set(LIBNEST2D_LIBRARIES ${LIBNEST2D_LIBRARIES} PARENT_SCOPE)
#endif()
