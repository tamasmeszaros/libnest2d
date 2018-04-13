#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <binpack2d.h>
#include <geometries_io.hpp>


TEST(BasicFunctionality, Angles)
{

    using namespace binpack2d;

    Degrees deg(180);
    Radians rad(deg);
    Degrees deg2(rad);

    ASSERT_DOUBLE_EQ(rad, Pi);
    ASSERT_DOUBLE_EQ(deg, 180);
    ASSERT_DOUBLE_EQ(deg2, 180);
    ASSERT_DOUBLE_EQ(rad, (Radians) deg);
    ASSERT_DOUBLE_EQ( (Degrees) rad, deg);

    ASSERT_TRUE(rad == deg);

}

// Simple test, does not use gmock
TEST(BasicFunctionality, creationAndDestruction)
{
    using namespace binpack2d;

    Shape sh( { {0, 0}, {1, 0}, {1, 1}, {0, 1} } );

    ASSERT_EQ(sh.vertexCount(), 4);

    Rectangle rect(10, 10);

    double a = rect.area();

    ASSERT_EQ(a, 100);
}

// Simple test, does not use gmock
TEST(BasicFunctionality, arrangeRectangles)
{
    using namespace binpack2d;

    DummySelectionStrategy dms;

    std::vector<Rectangle> rects = { {40, 40}, {10, 10}, {20, 20} };

    dms.addItems(rects.begin(), rects.end());

    auto nx0 = dms.nextGroup();

    ASSERT_EQ(nx0.size(), 1);

    ASSERT_DOUBLE_EQ(nx0[0].get().area(), 10*10);


    auto nx1 = dms.nextGroup();

    ASSERT_EQ(nx1.size(), 1);

    ASSERT_DOUBLE_EQ(nx1[0].get().area(), 20*20);


    auto nx2 = dms.nextGroup();

    ASSERT_EQ(nx2.size(), 1);

    ASSERT_DOUBLE_EQ(nx2[0].get().area(), 40*40);

}

void arrangeRectangles() {
    using namespace binpack2d;


    DummyArranger arr;

    std::vector<Rectangle> rects = { {40, 40}, {10, 10}, {20, 20}  };

    Rectangle bin = {100, 100};

    for(auto& rect : rects) { std::cout << rect << std::endl; }

    arr.arrange(rects.begin(), rects.end(), bin);

}


int main(int argc, char **argv) {

//    arrangeRectangles();

//    return EXIT_SUCCESS;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
