find_package(Subversion QUIET REQUIRED)

set(URL_CLIPPER "https://svn.code.sf.net/p/polyclipping/code/trunk/cpp" 
CACHE STRING "Clipper source code repository location.")

message(STATUS "Clipper not found so it will be downloaded.")
# Silently download and build the library in the build dir

include(DownloadProject)
download_project(  PROJ                clipper_library
                   SVN_REPOSITORY      ${URL_CLIPPER}
                   SVN_REVISION        -r540
                   #SOURCE_SUBDIR       cpp
                   PATCH_COMMAND       ${Subversion_SVN_EXECUTABLE} cleanup && ${Subversion_SVN_EXECUTABLE} patch 
                                       ${PROJECT_SOURCE_DIR}/external/clipper/clipper_fix.patch
                   CMAKE_ARGS         
                     -DCMAKE_INSTALL_PREFIX=${LIBNEST2D_DEP_DIR}
                     -DBUILD_SHARED_LIBS=OFF
)
