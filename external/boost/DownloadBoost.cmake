#include(DownloadProject)
include(${CMAKE_CURRENT_LIST_DIR}/../../cmake_modules/DownloadProject.cmake)

set(URL_BOOST "https://github.com/boostorg/geometry.git" CACHE STRING "Location of the boost sources")

#INSTALL_COMMAND $<"">   # b2 does that already
download_project(   PROJ               boost
                    GIT_REPOSITORY     ${URL_BOOST}
                    INSTALL_DIR        ${LIBNEST2D_DEP_DIR}
                    CONFIGURE_COMMAND  <1:"">
                    BUILD_COMMAND      <1:"">
                    INSTALL_COMMAND    "${CMAKE_COMMAND} -E copy_directory include/boost ${LIBNEST2D_DEP_DIR}/boost"
                    TEST_COMMAND       <1:"">
)