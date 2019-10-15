# Find NLopt library.
# The following variables are set
#
# NLopt_FOUND
# NLopt_INCLUDE_DIRS
# NLopt_LIBRARIES
#
# It searches the environment variable $NLopt_PATH automatically.

unset(NLopt_FOUND CACHE)
unset(NLopt_INCLUDE_DIRS CACHE)
unset(NLopt_LIBRARIES CACHE)
unset(NLopt_LIBRARIES_RELEASE CACHE)
unset(NLopt_LIBRARIES_DEBUG CACHE)

FIND_PATH(NLopt_INCLUDE_DIRS nlopt.hpp
    $ENV{NLopt_PATH}
    $ENV{NLopt_PATH}/cpp/
    $ENV{NLopt_PATH}/include/
    ${CMAKE_PREFIX_PATH}/include/nlopt
    ${CMAKE_PREFIX_PATH}/include/
    /opt/local/include/
    /opt/local/include/nlopt/
    /usr/local/include/
    /usr/local/include/nlopt/
    /usr/include
    /usr/include/nlopt/)

set(LIB_SEARCHDIRS 
    $ENV{NLopt_PATH}
    $ENV{NLopt_PATH}/cpp/
    $ENV{NLopt_PATH}/cpp/build/
    $ENV{NLopt_PATH}/lib/
    $ENV{NLopt_PATH}/lib/nlopt/
    ${CMAKE_PREFIX_PATH}/lib/
    ${CMAKE_PREFIX_PATH}/lib/nlopt/
    /opt/local/lib/
    /opt/local/lib/nlopt/
    /usr/local/lib/
    /usr/local/lib/nlopt/
    /usr/lib/nlopt
)

set(_deb_postfix "d")

FIND_LIBRARY(NLopt_LIBRARIES_RELEASE nlopt ${LIB_SEARCHDIRS})
FIND_LIBRARY(NLopt_LIBRARIES_DEBUG nlopt${_deb_postfix} ${LIB_SEARCHDIRS})

set(NLopt_LIBRARIES "")
if(NLopt_LIBRARIES_RELEASE)
    list(APPEND NLopt_LIBRARIES ${NLopt_LIBRARIES_RELEASE})
else()
    list(APPEND NLopt_LIBRARIES ${NLopt_LIBRARIES_DEBUG})
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(NLopt
    "NLopt library cannot be found. Consider set NLopt_PATH environment variable"
    NLopt_INCLUDE_DIRS
    NLopt_LIBRARIES)

MARK_AS_ADVANCED(
    NLopt_INCLUDE_DIRS
    NLopt_LIBRARIES)

if(NLopt_FOUND)
    add_library(NLopt::nlopt UNKNOWN IMPORTED)
    set_target_properties(NLopt::nlopt PROPERTIES IMPORTED_LOCATION ${NLopt_LIBRARIES})
    set_target_properties(NLopt::nlopt PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${NLopt_INCLUDE_DIRS})
    set_target_properties(NLopt::nlopt PROPERTIES
        IMPORTED_LOCATION_DEBUG          ${NLopt_LIBRARIES_DEBUG}
        IMPORTED_LOCATION_RELWITHDEBINFO ${NLopt_LIBRARIES_RELEASE}
        IMPORTED_LOCATION_RELEASE        ${NLopt_LIBRARIES_RELEASE}
        IMPORTED_LOCATION_MINSIZEREL     ${NLopt_LIBRARIES_RELEASE}
    )
endif()
