include(ExternalProject)
ExternalProject_Add(   
    rp_Boost
    URL "https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz"
    URL_HASH SHA256=882b48708d211a5f48e60b0124cf5863c1534cd544ecd0664bb534a4b5d506e9
    CONFIGURE_COMMAND  ""
    BUILD_COMMAND      ""
    BUILD_IN_SOURCE    ON
    INSTALL_COMMAND    ${CMAKE_COMMAND} -E copy_directory boost ${CMAKE_INSTALL_PREFIX}/include/boost
    TEST_COMMAND       ""
)
