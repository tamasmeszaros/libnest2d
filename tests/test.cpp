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

    Item sh = { {0, 0}, {1, 0}, {1, 1}, {0, 1} };

    ASSERT_EQ(sh.vertexCount(), 4);

    Item sh2 ({ {0, 0}, {1, 0}, {1, 1}, {0, 1} });

    ASSERT_EQ(sh2.vertexCount(), 4);

    // copy
    Item sh3 = sh2;

    ASSERT_EQ(sh3.vertexCount(), 4);

    sh2 = {};

    ASSERT_EQ(sh2.vertexCount(), 0);
    ASSERT_EQ(sh3.vertexCount(), 4);

}

TEST(GeometryAlgorithms, Distance) {
    using namespace binpack2d;

    Point p1 = {0, 0};

    Point p2 = {10, 0};
    Point p3 = {10, 10};

    ASSERT_DOUBLE_EQ(PointLike::distance(p1, p2), 10);
    ASSERT_DOUBLE_EQ(PointLike::distance(p1, p3), sqrt(200));

    Segment seg(p1, p3);

    auto val = PointLike::horizontalDistance(p2, seg).first;

    if(std::is_floating_point<Coord>::value)
        ASSERT_DOUBLE_EQ(static_cast<double>(val), 10);
    else
        ASSERT_EQ(val, 10);

    ASSERT_DOUBLE_EQ(PointLike::distance(p2, seg), 7.0710678118654755);
}

TEST(GeometryAlgorithms, Area) {
    using namespace binpack2d;

    Rectangle rect(10, 10);

    ASSERT_EQ(rect.area(), 100);

    Rectangle rect2 = {100, 100};

    ASSERT_EQ(rect2.area(), 10000);

}

TEST(GeometryAlgorithms, IsPointInsidePolygon) {
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
TEST(GeometryAlgorithms, LeftAndDownPolygon)
{
    using namespace binpack2d;

    Box bin(100, 100);
    BottomLeftPlacementStrategy placer(bin);

    Item item = {{70, 75}, {88, 60}, {65, 50}, {60, 30}, {80, 20}, {42, 20},
                 {35, 35}, {35, 55}, {40, 75}, {70, 75}};

    Item leftControl = { {40, 75}, {35, 55}, {35, 35}, {42, 20}, {0, 20},
                         {0, 75}, {40, 75}};

    Item downControl = {{88, 60},
                        {65, 50},
                        {60, 30},
                        {80, 20},
                        {42, 20},
                        {35, 35},
                        {0, 35},
                        {0, 88},
                        {88, 60}};

    Item leftp = placer.leftPoly(item);

    ASSERT_EQ(leftp.vertexCount(), leftControl.vertexCount());

    for(size_t i = 0; i < leftControl.vertexCount(); i++) {
        ASSERT_EQ(getX(leftp.vertex(i)), getX(leftControl.vertex(i)));
        ASSERT_EQ(getY(leftp.vertex(i)), getY(leftControl.vertex(i)));
    }

    Item downp = placer.downPoly(item);

    ASSERT_EQ(downp.vertexCount(), downControl.vertexCount());

    for(size_t i = 0; i < downControl.vertexCount(); i++) {
        ASSERT_EQ(getX(downp.vertex(i)), getX(downControl.vertex(i)));
        ASSERT_EQ(getY(downp.vertex(i)), getY(downControl.vertex(i)));
    }
}

// Simple test, does not use gmock
TEST(GeometryAlgorithms, ArrangeRectangles)
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

//    Rectangle bin(100, 100);
//    Rectangle bin2(10, 10);

//    bool inr = ShapeLike::intersects(bin.rawShape(), bin2.rawShape());

//    std::cout << inr << std::endl;

    Box bin(100, 100);
    BottomLeftPlacementStrategy placer(bin);

    Rectangle rect(20, 80);

    placer.pack(rect);

    Item item = {{70, 75}, {88, 60}, {65, 50}, {60, 30}, {80, 20}, {42, 20},
                 {35, 35}, {35, 55}, {40, 75}, {70, 75}};

    auto d = placer.availableSpaceLeft(item);

    std::cout << d << std::endl;

    std::string message;
    boost::geometry::is_valid(item.rawShape(), message);

    std::cout << message << std::endl;

//    boost::geometry::is_valid(bin.rawShape(), message);

//    std::cout << message << std::endl;

//    auto leftp = placer.leftPoly(item);

//    boost::geometry::is_valid(leftp, message);

//    std::cout << message << std::endl;

//    std::cout << ShapeLike::toString(leftp) << std::endl;

//    std::cout << "\n\n\n" << std::endl;
//    auto downp = placer.downPoly(item);

//    boost::geometry::is_valid(downp, message);

//    std::cout << message << std::endl;

//    std::cout << ShapeLike::toString(downp) << std::endl;


//    std::cout << ShapeLike::area(downp) << std::endl;

}


int main(int argc, char **argv) {
    arrangeRectangles();
//    return EXIT_SUCCESS;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
