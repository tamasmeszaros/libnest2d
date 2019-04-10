find_package(Armadillo REQUIRED)

add_library(optimlibOptimizer INTERFACE)
target_include_directories(optimlibOptimizer INTERFACE ${ARMADILLO_INCLUDE_DIRS})
target_link_libraries(optimlibOptimizer INTERFACE ${ARMADILLO_LIBRARIES})