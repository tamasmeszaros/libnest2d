include(ExternalProject)

if (BUILD_SHARED_LIBS)
    set(_TBB_BUILD_STATIC OFF)
else()
    set(_TBB_BUILD_STATIC ON)
endif()

ExternalProject_Add(
    rp_TBB
    GIT_REPOSITORY      https://github.com/wjakob/tbb.git
    GIT_TAG             b066def
    INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}
    BINARY_DIR          ${RP_PACKAGE_BUILD_DIR}
    CMAKE_ARGS         
        -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
        -DTBB_BUILD_STATIC=${_TBB_BUILD_STATIC}
        -DTBB_BUILD_SHARED=${BUILD_SHARED_LIBS}
        -DTBB_BUILD_TESTS=OFF
        -DCMAKE_DEBUG_POSTFIX=_debug
    UPDATE_COMMAND ""
)

