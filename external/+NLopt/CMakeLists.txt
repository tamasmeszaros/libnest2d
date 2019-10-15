include(ExternalProject)
Externalproject_Add( 
  rp_NLopt
  GIT_REPOSITORY      https://github.com/stevengj/nlopt.git
  GIT_TAG             v2.5.0
  INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}
  CMAKE_ARGS         
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} 
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DNLOPT_PYTHON:BOOL=OFF
    -DNLOPT_OCTAVE:BOOL=OFF
    -DNLOPT_MATLAB:BOOL=OFF
    -DNLOPT_GUILE:BOOL=OFF
    -DNLOPT_SWIG:BOOL=OFF 
    -DCMAKE_DEBUG_POSTFIX=d
)

get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_is_multi)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/configs.txt "Release;Debug")
endif()