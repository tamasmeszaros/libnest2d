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

    Shape sh = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

    ASSERT_EQ(sh.vertexCount(), 4);

    Shape sh2 ({ {0, 0}, {1, 0}, {1, 1}, {0, 1} });

    ASSERT_EQ(sh2.vertexCount(), 4);

    // copy
    Shape sh3 = sh2;

    ASSERT_EQ(sh3.vertexCount(), 4);

    sh2 = {};

    ASSERT_EQ(sh2.vertexCount(), 0);
    ASSERT_EQ(sh3.vertexCount(), 4);

}

TEST(GeometryAlgorithms, Area) {
    using namespace binpack2d;

    Rectangle rect(10, 10);

    ASSERT_EQ(rect.area(), 100);

    Rectangle rect2 = {100, 100};

    ASSERT_EQ(rect2.area(), 10000);

}

TEST(GeometryAlgorithms, isPointInsidePolygon) {
    using namespace binpack2d;

    Rectangle rect(10, 10);

    Point p = {1, 1};

    ASSERT_TRUE(rect.isPointInside(p));

    p = {11, 11};

    ASSERT_FALSE(rect.isPointInside(p));


    p = {11, 12};

    ASSERT_FALSE(rect.isPointInside(p));


    p = {3, 3};

    ASSERT_TRUE(rect.isPointInside(p));

}

// Simple test, does not use gmock
TEST(GeometryAlgorithms, arrangeRectangles)
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


    Point p1 = {0, 0};
    Point p2 = {10, 10};

//    std::cout << PointLike::distance<Point>(p1, p2) << std::endl;

    Shape sh1 = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };
//    Shape sh2 = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

    std::cout << sh1 << std::endl;

//    bool ret = Shape::intersects(sh1, sh2);
//    std::cout << ret << std::endl;

//    DummyArranger arr;

//    std::vector<Rectangle> rects = { {40, 40}, {10, 10}, {20, 20}  };

//    Rectangle bin = {100, 100};

//    for(auto& rect : rects) { std::cout << rect << std::endl; }

//    arr.arrange(rects.begin(), rects.end(), bin);

}


int main(int argc, char **argv) {
//    arrangeRectangles();
//    return EXIT_SUCCESS;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
