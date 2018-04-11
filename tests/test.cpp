#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <Binpack2D/binpack2d.hpp>
#include <Binpack2D/geometries_io.hpp>
#include <Binpack2DBridge/binpack2dbridge.hpp>

#include <Point.hpp>

// Simple test, does not use gmock
TEST(BasicFunctionality, creationAndDestruction)
{

    Slic3r::Point p(10, 10);



    binpack2d::Shape<Slic3r::Polygon> sh( Slic3r::Polygon({ {0, 0}, {1, 0}, {1, 1}, {0, 1} }) );

    std::cout << sh << std::endl;

    ASSERT_EQ(binpack2d::PointLike::x(p), 10);

//    binpack2d::Rectangle rect(10, 10);

//    std::cout << rect << std::endl;

//    double a = rect.area();

//    ASSERT_EQ(a, 100);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
