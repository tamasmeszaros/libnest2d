rp_add_cmake_project(GTest
    GIT_REPOSITORY      https://github.com/google/googletest.git
    GIT_TAG             release-1.10.0
    CMAKE_ARGS
        -DBUILD_GTEST:BOOL=ON
        -DBUILD_GMOCK:BOOL=OFF
)
