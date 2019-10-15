require_package(OpenMP REQUIRED)

add_library(ompThreading INTERFACE)
target_link_libraries(ompThreading INTERFACE OpenMP::OpenMP_CXX)
install(TARGETS ompThreading EXPORT Libnest2DTargets INCLUDES DESTINATION include)
