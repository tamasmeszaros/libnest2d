
add_executable(example     ${PROJECT_SOURCE_DIR}/examples/main.cpp
#                           tools/libnfpglue.hpp
#                           tools/libnfpglue.cpp
                       ${PROJECT_SOURCE_DIR}/tools/nfp_svgnest.hpp
                       ${PROJECT_SOURCE_DIR}/tools/nfp_svgnest_glue.hpp
                       ${PROJECT_SOURCE_DIR}/tools/svgtools.hpp
                       ${PROJECT_SOURCE_DIR}/tools/printer_parts.cpp
                       ${PROJECT_SOURCE_DIR}/tools/printer_parts.h
                       )

if(NOT LIBNEST2D_HEADER_ONLY)
target_link_libraries(example ${LIBNAME})
else()
target_link_libraries(example libnest2d)
endif()
