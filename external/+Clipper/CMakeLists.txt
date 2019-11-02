include(ExternalProject)

require_dependency(Boost)

ExternalProject_Add(
    rp_Clipper
    GIT_REPOSITORY      https://github.com/tamasmeszaros/libpolyclipping.git
    INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}
    BINARY_DIR          ${RP_PACKAGE_BUILD_DIR}
    CMAKE_ARGS
     -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
     -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
     -DCMAKE_DEBUG_POSTFIX=d
     -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
    UPDATE_COMMAND ""
)
