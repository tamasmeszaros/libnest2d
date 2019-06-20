find_package(Subversion QUIET REQUIRED)

set(URL_CLIPPER "https://svn.code.sf.net/p/polyclipping/code/trunk/cpp" 
CACHE STRING "Clipper source code repository location.")

include(ExternalProject)
ExternalProject_Add(dep_clipper
                   SVN_REPOSITORY      ${URL_CLIPPER}
                   SVN_REVISION        -r540
		   INSTALL_DIR	       ${CMAKE_PREFIX_PATH}
                   #SOURCE_SUBDIR       cpp
                   PATCH_COMMAND       ${Subversion_SVN_EXECUTABLE} cleanup && ${Subversion_SVN_EXECUTABLE} patch 
                                       ${CMAKE_CURRENT_LIST_DIR}clipper_fix.patch
                   CMAKE_ARGS
                     -DBUILD_SHARED_LIBS=OFF
)
