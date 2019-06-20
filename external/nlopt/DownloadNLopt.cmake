include(DownloadProject)

set(URL_NLOPT "https://github.com/stevengj/nlopt.git" 
CACHE STRING "Location of the nlopt git repository")

download_project( PROJ                nlopt
                  GIT_REPOSITORY      ${URL_NLOPT}
                  GIT_TAG             v2.5.0
                  INSTALL_DIR         ${LIBNEST2D_DEP_DIR}
                  CMAKE_ARGS         
                    -DCMAKE_INSTALL_PREFIX=${LIBNEST2D_DEP_DIR}
                    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} 
                    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
                    -DNLOPT_PYTHON:BOOL=OFF
                    -DNLOPT_OCTAVE:BOOL=OFF
                    -DNLOPT_MATLAB:BOOL=OFF
                    -DNLOPT_GUILE:BOOL=OFF
                    -DNLOPT_SWIG:BOOL=OFF 
                    -DNLOPT_LINK_PYTH:BOOL=OFF
)
