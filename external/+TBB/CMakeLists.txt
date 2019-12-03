set (_tbb_confs "")
if (BUILD_SHARED_LIBS)
    set(_tbb_confs -DTBB_BUILD_STATIC:BOOL=OFF -DTBB_BUILD_SHARED:BOOL=ON)
else()
    set(_tbb_confs -DTBB_BUILD_STATIC:BOOL=ON -DTBB_BUILD_SHARED:BOOL=OFF)
endif()

rp_add_cmake_project(
    TBB
    #GIT_REPOSITORY      https://github.com/wjakob/tbb.git
    #GIT_TAG             20357d83871e4cb93b2c724fe0c337cd999fd14f
    URL "https://github.com/wjakob/tbb/archive/a0dc9bf76d0120f917b641ed095360448cabc85b.tar.gz"
    URL_HASH SHA256=0545cb6033bd1873fcae3ea304def720a380a88292726943ae3b9b207f322efe
    CMAKE_ARGS          
        ${_tbb_confs}
        -DTBB_BUILD_TESTS=OFF
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_DEBUG_POSTFIX=_debug
)

