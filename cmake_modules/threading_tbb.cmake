find_package(TBB QUIET)

add_library(tbbThreading INTERFACE)

if(NOT TBB_FOUND)

    message(STATUS  "TBB requested but not found so downloading "
                    "and automatic build is performed...")

    include(RequirePackage)

    require_package(tbb)

    # Now the library is downloaded, configured, built and installed and the find
    # command should not have any problem to find it. 
    set(TBB_STATIC ON)
    find_package(TBB REQUIRED)

else()
    message(STATUS "Intel TBB found: TRUE")
endif()

if(MSVC)
    # Suppress implicit linking of the TBB libraries by the Visual Studio compiler.
    target_compile_definitions(tbbThreading INTERFACE -D__TBB_NO_IMPLICIT_LINKAGE)
endif()

target_link_libraries(tbbThreading INTERFACE TBB::tbb)
target_compile_definitions(tbbThreading INTERFACE -DTBB_USE_CAPTURED_EXCEPTION=0)
