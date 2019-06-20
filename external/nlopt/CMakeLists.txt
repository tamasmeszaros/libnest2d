set(RP_URL_NLOPT "https://github.com/stevengj/nlopt.git" 
CACHE STRING "Location of the nlopt git repository")

include(ExternalProject)
Externalproject_Add( 
  rp_nlopt
  GIT_REPOSITORY      ${RP_URL_NLOPT}
  GIT_TAG             v2.5.0
  INSTALL_DIR         ${RP_INSTALL_PREFIX}
  CMAKE_ARGS         
    -DCMAKE_INSTALL_PREFIX=${RP_INSTALL_PREFIX}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} 
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
    -DNLOPT_PYTHON:BOOL=OFF
    -DNLOPT_OCTAVE:BOOL=OFF
    -DNLOPT_MATLAB:BOOL=OFF
    -DNLOPT_GUILE:BOOL=OFF
    -DNLOPT_SWIG:BOOL=OFF 
    -DNLOPT_LINK_PYTH:BOOL=OFF
)
