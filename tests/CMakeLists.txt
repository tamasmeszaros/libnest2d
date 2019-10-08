require_package(GTest 1.7 REQUIRED)

add_executable(tests_clipper_nlopt
    test.cpp
    ../tools/svgtools.hpp
#   ../tools/libnfpglue.hpp
#  ../tools/libnfpglue.cpp
    ../tools/printer_parts.h
    ../tools/printer_parts.cpp
)

target_link_libraries(tests_clipper_nlopt libnest2d GTest::GTest GTest::Main)

if(NOT LIBNEST2D_HEADER_ONLY)
    target_link_libraries(tests_clipper_nlopt ${LIBNAME})
else()
    target_link_libraries(tests_clipper_nlopt libnest2d)
endif()

target_include_directories(tests_clipper_nlopt BEFORE PRIVATE ${GTEST_INCLUDE_DIRS})

if(NOT LIBNEST2D_HEADER_ONLY)
    target_link_libraries(tests_clipper_nlopt ${LIBNAME})
else()
    target_link_libraries(tests_clipper_nlopt libnest2d)
endif()

add_test(libnest2d_tests tests_clipper_nlopt)
