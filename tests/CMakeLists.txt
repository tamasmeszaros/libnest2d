require_package(Catch2 2.9.1 REQUIRED)

add_executable(tests_clipper_nlopt
    test.cpp
    ../tools/svgtools.hpp
#   ../tools/libnfpglue.hpp
#  ../tools/libnfpglue.cpp
    ../tools/printer_parts.hpp
    ../tools/printer_parts.cpp
)

target_link_libraries(tests_clipper_nlopt libnest2d Catch2::Catch2)

if(NOT LIBNEST2D_HEADER_ONLY)
    target_link_libraries(tests_clipper_nlopt ${LIBNAME})
else()
    target_link_libraries(tests_clipper_nlopt libnest2d)
endif()

add_test(libnest2d_tests tests_clipper_nlopt)
