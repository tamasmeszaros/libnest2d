include(ExternalProject)
ExternalProject_Add(
    rp_Catch2
    URL "https://github.com/catchorg/Catch2/archive/v2.9.1.tar.gz"
    URL_HASH SHA256=0b36488aca6265e7be14da2c2d0c748b4ddb9c70a1ea4da75736699c629f14ac
    INSTALL_DIR     ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCATCH_INSTALL_DOCS=OFF 
        -DCATCH_BUILD_TESTING=OFF
        -DCATCH_INSTALL_HELPERS=ON
)
