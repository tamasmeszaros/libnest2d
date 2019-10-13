include(ExternalProject)

if (BUILD_SHARED_LIBS)
    set(_TBB_BULD_STATIC OFF)
else()
    set(_TBB_BUILD_STATIV ON)
endif()

ExternalProject_Add(
    rp_TBB
    GIT_REPOSITORY      https://github.com/wjakob/tbb.git
    GIT_TAG             b066def
    INSTALL_DIR         ${RP_INSTALL_PREFIX}
    CMAKE_ARGS         
        -DCMAKE_INSTALL_PREFIX=${RP_INSTALL_PREFIX}
        -DTBB_BUILD_STATIC=${_TBB_BULD_STATIC}
        -DTBB_BUILD_SHARED=${BUILD_SHARED_LIBS}
        $<$<CONFIG:Debug>:-DCMAKE_DEBUG_POSTFIX=${RP_DEBUG_POSTFIX}>
)
