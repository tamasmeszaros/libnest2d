find_package(Threads REQUIRED)

add_library(stdThreading INTERFACE)
target_link_libraries(stdThreading INTERFACE Threads::Threads)
