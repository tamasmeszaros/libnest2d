require_package(Catch2 2.9.1 REQUIRED)

add_executable(tests_clipper_nlopt
    test.cpp
    ../tools/svgtools.hpp
#   ../tools/libnfpglue.hpp
#  ../tools/libnfpglue.cpp
    ../tools/printer_parts.hpp
    ../tools/printer_parts.cpp
)

if (MSVC)
# with catch, it is to much
target_compile_options(tests_clipper_nlopt PRIVATE /bigobj)
endif ()

target_link_libraries(tests_clipper_nlopt libnest2d Catch2::Catch2)
add_test(libnest2d_tests tests_clipper_nlopt)
