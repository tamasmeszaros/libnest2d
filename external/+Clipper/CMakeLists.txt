find_package(Subversion QUIET REQUIRED)
include(ExternalProject)
ExternalProject_Add(
    rp_Clipper
    SVN_REPOSITORY      https://svn.code.sf.net/p/polyclipping/code/trunk/cpp
    SVN_REVISION        -r540
    INSTALL_DIR         ${RP_INSTALL_PREFIX}
    PATCH_COMMAND       ${Subversion_SVN_EXECUTABLE} cleanup && ${Subversion_SVN_EXECUTABLE} patch ${CMAKE_CURRENT_LIST_DIR}/clipper_fix.patch
    CMAKE_ARGS
     -DBUILD_SHARED_LIBS:BOOL=OFF
     $<$<CONFIG:Debug>:-DCMAKE_DEBUG_POSTFIX=${RP_DEBUG_POSTFIX}>
     -DCMAKE_INSTALL_PREFIX:PATH=${RP_INSTALL_PREFIX}
)
