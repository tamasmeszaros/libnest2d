require_package(Catch2 2.9.1 REQUIRED NO_EXPORT)

add_executable(tests_clipper_nlopt
    test.cpp
    ../tools/svgtools.hpp
#   ../tools/libnfpglue.hpp
#  ../tools/libnfpglue.cpp
    ../tools/printer_parts.hpp
    ../tools/printer_parts.cpp
)

target_link_libraries(tests_clipper_nlopt libnest2d Catch2::Catch2)
add_test(libnest2d_tests tests_clipper_nlopt)
