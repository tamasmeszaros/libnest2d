include(DownloadProject)

set(URL_TBB "https://github.com/wjakob/tbb.git" 
CACHE STRING "Location of the tbb git repository")

download_project(   PROJ                tbb
                GIT_REPOSITORY      ${URL_TBB}
                GIT_TAG             b066def
                INSTALL_DIR         ${LIBNEST2D_DEP_DIR}
                CMAKE_ARGS         
                    -DCMAKE_INSTALL_PREFIX=${LIBNEST2D_DEP_DIR}
                    -DTBB_BUILD_STATIC=ON
                    -DTBB_BUILD_SHARED=OFF
)
