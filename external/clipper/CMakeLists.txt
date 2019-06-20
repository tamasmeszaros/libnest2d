find_package(Subversion QUIET REQUIRED)

set(RP_URL_CLIPPER "https://svn.code.sf.net/p/polyclipping/code/trunk/cpp" 
CACHE STRING "Clipper source code repository location.")

include(ExternalProject)
ExternalProject_Add(
    rp_clipper
    SVN_REPOSITORY      ${RP_URL_CLIPPER}
    SVN_REVISION        -r540
    INSTALL_DIR         ${RP_INSTALL_PREFIX}
    PATCH_COMMAND       ${Subversion_SVN_EXECUTABLE} cleanup && ${Subversion_SVN_EXECUTABLE} patch ${CMAKE_CURRENT_LIST_DIR}/clipper_fix.patch
    CMAKE_ARGS
     -DBUILD_SHARED_LIBS:BOOL=OFF
     -DCMAKE_INSTALL_PREFIX:PATH=${RP_INSTALL_PREFIX}
)
