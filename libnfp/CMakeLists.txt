
find_package(Boost REQUIRED)

if(NOT Boost_FOUND)
    # Silently download the required boost header sources

    if (CMAKE_VERSION VERSION_LESS 3.2)
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
    else()
        set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
    endif()

    include(DownloadProject)
    download_project(PROJ                boostgeometry
                GIT_REPOSITORY      https://github.com/boostorg/geometry.git
                GIT_TAG             boost-1.64.0
                ${UPDATE_DISCONNECTED_IF_AVAILABLE}
    )

    download_project(PROJ                boostmultiprecision
                GIT_REPOSITORY      https://github.com/boostorg/multiprecision.git
                GIT_TAG             boost-1.64.0
                ${UPDATE_DISCONNECTED_IF_AVAILABLE}
    )
    set(BOOST_HEADERS ${boostgeometry_SOURCE_DIR}/include
                      ${boostmultiprecision_SOURCE_DIR}/include)
else()
    set(BOOST_HEADERS ${Boost_INCLUDE_DIRS})
endif()

set(LIBNFP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/libnfp.hpp PARENT_SCOPE)

set(LIBNFP_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}
                    ${BOOST_HEADERS}
    PARENT_SCOPE
    )

 set(LIBNFP_LIBRARIES ${Boost_LIBRARIES} PARENT_SCOPE)
