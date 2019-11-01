find_package(Threads REQUIRED)

add_library(stdThreading INTERFACE)
target_link_libraries(stdThreading INTERFACE Threads::Threads)

install(TARGETS stdThreading EXPORT Libnest2DTargets INCLUDES DESTINATION include)
set(LIBNEST2D_PUBLIC_PACKAGES "${LIBNEST2D_PUBLIC_PACKAGES};Threads" CACHE INTERNAL "")
