#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <Binpack2D/binpack2d.hpp>
#include <Binpack2D/geometries_io.hpp>
#include <Binpack2DBridge/binpack2dbridge.hpp>

#include <Point.hpp>
#include <Polygon.hpp>


// Simple test, does not use gmock
TEST(BasicFunctionality, creationAndDestruction)
{

    binpack2d::Rectangle rect(10, 10);

    std::cout << rect << std::endl;

    double a = rect.area();

    ASSERT_EQ(a, 100);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
