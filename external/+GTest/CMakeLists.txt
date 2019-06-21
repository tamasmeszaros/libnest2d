include(ExternalProject)
set(RP_URL_GTEST "https://github.com/google/googletest.git" 
    CACHE STRING "Google test source code repository location.")
        
ExternalProject_Add(
    rp_GTest
    GIT_REPOSITORY      ${RP_URL_GTEST}
    GIT_TAG             release-1.8.0
    INSTALL_DIR         ${RP_INSTALL_PREFIX}
    CMAKE_ARGS         
        -DCMAKE_INSTALL_PREFIX=${RP_INSTALL_PREFIX}
        -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS} 
        -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE} 
        -DBUILD_GTEST:BOOL=ON
        -DBUILD_GMOCK:BOOL=OFF
        -DCMAKE_DEBUG_POSTFIX=${RP_DEBUG_POSTFIX}
)
