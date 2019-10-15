find_package(Subversion QUIET REQUIRED)
include(ExternalProject)
ExternalProject_Add(
    rp_Clipper
    SVN_REPOSITORY      https://svn.code.sf.net/p/polyclipping/code/trunk/cpp
    SVN_REVISION        -r540
    INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}
    PATCH_COMMAND       ${Subversion_SVN_EXECUTABLE} cleanup . && ${Subversion_SVN_EXECUTABLE} patch ${CMAKE_CURRENT_LIST_DIR}/clipper_fix.patch
    CMAKE_ARGS
     -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
     -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
     $<$<CONFIG:Debug>:-DCMAKE_DEBUG_POSTFIX=d>
     -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_is_multi)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/configs.txt "Release;Debug")
endif()
