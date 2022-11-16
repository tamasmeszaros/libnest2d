include(ExternalProject)

if (WIN32)
    set(_bootstrap_cmd bootstrap.bat)
    set(_build_cmd  b2.exe)
else()
    set(_bootstrap_cmd ./bootstrap.sh)
    set(_build_cmd ./b2)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(_boost_toolset gcc)
    configure_file(${CMAKE_CURRENT_LIST_DIR}/user-config.jam boost-user-config.jam)
    set(_patch_command ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/boost-user-config.jam ./tools/build/src/tools/user-config.jam)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # https://cmake.org/cmake/help/latest/variable/MSVC_VERSION.html
    if (MSVC_VERSION EQUAL 1800)
    # 1800      = VS 12.0 (v120 toolset)
        set(_boost_toolset "msvc-12.0")
    elseif (MSVC_VERSION EQUAL 1900)
    # 1900      = VS 14.0 (v140 toolset)
        set(_boost_toolset "msvc-14.0")
    elseif (MSVC_VERSION LESS 1920)
    # 1910-1919 = VS 15.0 (v141 toolset)
        set(_boost_toolset "msvc-14.1")
    elseif (MSVC_VERSION LESS 1930)
    # 1920-1929 = VS 16.0 (v142 toolset)
        set(_boost_toolset "msvc-14.2")
    elseif (MSVC_VERSION LESS 1940)
    # 1930-1939 = VS 17.0 (v143 toolset)
        set(_boost_toolset "msvc-14.3")
    else ()
        message(FATAL_ERROR "Unsupported MSVC version")
    endif ()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    if (WIN32)
        set(_boost_toolset "clang-win")
    elseif (APPLE)
        set(_boost_toolset "clang")
    else()
        set(_boost_toolset clang)
        configure_file(${CMAKE_CURRENT_LIST_DIR}/user-config.jam boost-user-config.jam)
        set(_patch_command ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/boost-user-config.jam ./tools/build/src/tools/user-config.jam)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(_boost_toolset "intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(_boost_toolset "clang")
endif()

if (RP_PACKAGE STREQUAL "Boost")
message(STATUS "Deduced boost toolset: ${_boost_toolset} based on ${CMAKE_CXX_COMPILER_ID} compiler")
endif()

set(_libs "")
foreach(_comp ${RP_Boost_COMPONENTS})
    list(APPEND _libs "--with-${_comp}")
endforeach()

if (BUILD_SHARED_LIBS)
    set(_link shared)
else()
    set(_link static)
endif()

set(_bits "")
if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(_bits 64)
elseif ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "4")
    set(_bits 32)
endif ()

include(ProcessorCount)
ProcessorCount(NPROC)
file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX} _prefix)

set(_boost_flags "")
if (UNIX)
    set(_boost_flags "cflags=-fPIC;cxxflags=-fPIC")
endif ()

if(APPLE)
    set(_boost_flags 
        "cflags=-fPIC -mmacosx-version-min=${RP_OSX_TARGET};"
        "cxxflags=-fPIC -mmacosx-version-min=${RP_OSX_TARGET};"
        "mflags=-fPIC -mmacosx-version-min=${RP_OSX_TARGET};"
        "mmflags=-fPIC -mmacosx-version-min=${RP_OSX_TARGET}") 
endif()

set(_boost_variants "")
if(CMAKE_BUILD_TYPE)
    list(APPEND CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
    list(REMOVE_DUPLICATES CMAKE_CONFIGURATION_TYPES)
endif()
list(FIND CMAKE_CONFIGURATION_TYPES "Release" _cfg_rel)
list(FIND CMAKE_CONFIGURATION_TYPES "RelWithDebInfo" _cfg_relwdeb)
list(FIND CMAKE_CONFIGURATION_TYPES "MinSizeRel" _cfg_minsizerel)
list(FIND CMAKE_CONFIGURATION_TYPES "Debug" _cfg_deb)

if (_cfg_rel GREATER -1 OR _cfg_relwdeb GREATER -1 OR _cfg_minsizerel GREATER -1)
    list(APPEND _boost_variants release)
endif()

if (_cfg_deb GREATER -1)
    list(APPEND _boost_variants debug)
endif()

if (NOT _boost_variants)
    set(_boost_variants release)
endif()

set(_build_cmd ${_build_cmd}
               ${_boost_flags}
               -j${NPROC}
               ${_libs}
               --layout=versioned
               --debug-configuration
               toolset=${_boost_toolset}
               address-model=${_bits}
               link=${_link}
               threading=multi
               boost.locale.icu=off
               --disable-icu
               ${_boost_variants}
               stage)

set(_install_cmd ${_build_cmd} --prefix=${_prefix} install)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # When Clang is used with enabled UndefinedBehaviorSanitizer, it produces "undefined reference to '__muloti4'" when __int128 is used.
    # Because of that, UndefinedBehaviorSanitizer is disabled for those functions that use __int128.
    list(APPEND _patch_command COMMAND ${PATCH_CMD} ${CMAKE_CURRENT_LIST_DIR}/Boost.patch)
endif ()

ExternalProject_Add(
    rp_Boost
    URL "https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.zip"
    URL_HASH SHA256=f22143b5528e081123c3c5ed437e92f648fe69748e95fa6e2bd41484e2986cc3
    DOWNLOAD_EXTRACT_TIMESTAMP ON
    CONFIGURE_COMMAND "${_bootstrap_cmd}"
    PATCH_COMMAND ${_patch_command}
    BUILD_COMMAND "${_build_cmd}"
    BUILD_IN_SOURCE    ON
    INSTALL_COMMAND "${_install_cmd}"
)
