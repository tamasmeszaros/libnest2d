set(RP_URL_TBB "https://github.com/wjakob/tbb.git" CACHE STRING "Location of the tbb git repository")

include(ExternalProject)
ExternalProject_Add(
    rp_TBB
    GIT_REPOSITORY      ${RP_URL_TBB}
    GIT_TAG             b066def
    INSTALL_DIR         ${RP_INSTALL_PREFIX}
    CMAKE_ARGS         
        -DCMAKE_INSTALL_PREFIX=${RP_INSTALL_PREFIX}
        -DTBB_BUILD_STATIC=ON
        -DTBB_BUILD_SHARED=OFF
        -DCMAKE_DEBUG_POSTFIX=${RP_DEBUG_POSTFIX}
)
