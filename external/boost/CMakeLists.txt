set(RP_URL_BOOST "https://github.com/boostorg/geometry.git" CACHE STRING "Location of the boost sources")

include(ExternalProject)
ExternalProject_Add(   
    rp_boost
    GIT_REPOSITORY     ${RP_URL_BOOST}
    CONFIGURE_COMMAND  ""
    BUILD_COMMAND      ""
    BUILD_IN_SOURCE    ON
    INSTALL_COMMAND    ${CMAKE_COMMAND} -E copy_directory include/boost ${RP_INSTALL_PREFIX}/include/boost
    TEST_COMMAND       ""
)
