#set(RP_URL_BOOST "https://github.com/boostorg/geometry.git" CACHE STRING "Location of the boost sources")

include(ExternalProject)
ExternalProject_Add(   
    rp_Boost
    #GIT_REPOSITORY     ${RP_URL_BOOST}
    URL "https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz"
    URL_HASH SHA256=bd0df411efd9a585e5a2212275f8762079fed8842264954675a4fddc46cfcf60
    CONFIGURE_COMMAND  ""
    BUILD_COMMAND      ""
    BUILD_IN_SOURCE    ON
    INSTALL_COMMAND    ${CMAKE_COMMAND} -E copy_directory boost ${CMAKE_INSTALL_PREFIX}/include/boost
    TEST_COMMAND       ""
)
