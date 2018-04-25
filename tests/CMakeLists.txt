
# Try to find existing GTest installation
find_package(GTest QUIET)

if(NOT GTEST_FOUND)
    # Go and download google test framework, integrate it with the build
    set(GTEST_LIBRARIES gtest gmock)

    if (CMAKE_VERSION VERSION_LESS 3.2)
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
    else()
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
    endif()

    include(DownloadProject)
    download_project(PROJ                googletest
                     GIT_REPOSITORY      https://github.com/google/googletest.git
                     GIT_TAG             master
                     ${UPDATE_DISCONNECTED_IF_AVAILABLE}
    )

    # Prevent GoogleTest from overriding our compiler/linker options
    # when building with Visual Studio
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    add_subdirectory(${googletest_SOURCE_DIR}
                     ${googletest_BINARY_DIR}
                     )

else()
    include_directories(${GTEST_INCLUDE_DIRS} )
endif()

include_directories("${CMAKE_SOURCE_DIR}")
add_executable(bp2d_tests test.cpp printer_parts.h printer_parts.cpp)
target_link_libraries(bp2d_tests binpack2d
    ${GTEST_LIBRARIES}
)

if(DEFINED BINPACK2D_TEST_LIBRARIES)
    target_link_libraries(bp2d_tests ${BINPACK2D_TEST_LIBRARIES})
endif()

add_test(gtests bp2d_tests)


