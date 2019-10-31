include(ExternalProject)
ExternalProject_Add(
    rp_Clipper
    GIT_REPOSITORY      https://github.com/tamasmeszaros/libpolyclipping.git
    INSTALL_DIR         ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS
     -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
     -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
     -DCMAKE_DEBUG_POSTFIX=d
     -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX}
)

get_property(_is_multi GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (_is_multi)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/configs.txt "Release;Debug")
endif()
