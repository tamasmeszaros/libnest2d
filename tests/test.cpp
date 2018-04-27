#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>

#include <binpack2d.h>
#include "printer_parts.h"
#include <binpack2d/geometries_io.hpp>

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

    ASSERT_DOUBLE_EQ(PointLike::distance(p2, seg), 7.0710678118654755);

    auto result = PointLike::horizontalDistance(p2, seg);

    auto check = [](Coord val, Coord expected) {
        if(std::is_floating_point<Coord>::value)
            ASSERT_DOUBLE_EQ(static_cast<double>(val), expected);
        else
            ASSERT_EQ(val, expected);
    };

    ASSERT_TRUE(result.second);
    check(result.first, 10);

    result = PointLike::verticalDistance(p2, seg);
    ASSERT_TRUE(result.second);
    check(result.first, -10);

    result = PointLike::verticalDistance(Point{10, 20}, seg);
    ASSERT_TRUE(result.second);
    check(result.first, 10);


    Point p4 = {80, 0};
    Segment seg2 = { {0, 0}, {0, 40} };

    result = PointLike::horizontalDistance(p4, seg2);

    ASSERT_TRUE(result.second);
    check(result.first, 80);

    result = PointLike::verticalDistance(p4, seg2);
    // Point should not be related to the segment
    ASSERT_FALSE(result.second);

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

//TEST(GeometryAlgorithms, Intersections) {
//    using namespace binpack2d;

//    Rectangle rect(70, 30);

//    rect.translate({80, 60});

//    Rectangle rect2(80, 60);
//    rect2.translate({80, 0});

////    ASSERT_FALSE(Item::intersects(rect, rect2));

//    Segment s1({0, 0}, {10, 10});
//    Segment s2({1, 1}, {11, 11});
//    ASSERT_FALSE(ShapeLike::intersects(s1, s1));
//    ASSERT_FALSE(ShapeLike::intersects(s1, s2));
//}

// Simple test, does not use gmock
TEST(GeometryAlgorithms, LeftAndDownPolygon)
{
    using namespace binpack2d;

    Box bin(100, 100);
    BottomLeftPlacer placer(bin);

    Item item = {{70, 75}, {88, 60}, {65, 50}, {60, 30}, {80, 20}, {42, 20},
                 {35, 35}, {35, 55}, {40, 75}, {70, 75}};

    Item leftControl = { {40, 75},
                         {35, 55},
                         {35, 35},
                         {42, 20},
                         {0,  20},
                         {0,  75},
                         {40, 75}};

    Item downControl = {{88, 60},
                        {88, 0},
                        {35, 0},
                        {35, 35},
                        {42, 20},
                        {80, 20},
                        {60, 30},
                        {65, 50},
                        {88, 60}};

    Item leftp = placer.leftPoly(item);

    ASSERT_TRUE(ShapeLike::isValid(leftp).first);

    ASSERT_EQ(leftp.vertexCount(), leftControl.vertexCount());

    for(size_t i = 0; i < leftControl.vertexCount(); i++) {
        ASSERT_EQ(getX(leftp.vertex(i)), getX(leftControl.vertex(i)));
        ASSERT_EQ(getY(leftp.vertex(i)), getY(leftControl.vertex(i)));
    }

    Item downp = placer.downPoly(item);

    ASSERT_TRUE(ShapeLike::isValid(downp).first);

    ASSERT_EQ(downp.vertexCount(), downControl.vertexCount());

    for(size_t i = 0; i < downControl.vertexCount(); i++) {
        ASSERT_EQ(getX(downp.vertex(i)), getX(downControl.vertex(i)));
        ASSERT_EQ(getY(downp.vertex(i)), getY(downControl.vertex(i)));
    }

    BottomLeftPlacer::Config config;
    config.min_obj_distance = 2;
    placer.configure( config );

    Point tl = {40, 75};
    Coord d = config.min_obj_distance;
    tl += { 0, d};

    Point bl = {42, 20};
    bl += {0, -d};


    leftControl = {  tl,
                     {40, 75},
                     {35, 55},
                     {35, 35},
                     {42, 20},
                     bl,
                     {0,  getY(bl)},
                     {0,  getY(tl)},
                     tl};

    tl = {88, 60};
    tl += {d, 0};

    bl = {35, 35};
    bl += {-d, 0};

    downControl = {tl,
                    {getX(tl), 0},
                    {getX(bl), 0},
                    bl,
                   {35, 35},
                    {42, 20},
                    {80, 20},
                    {60, 30},
                    {65, 50},
                   {88, 60},
                    tl};

    leftp = placer.leftPoly(item);

    ASSERT_TRUE(ShapeLike::isValid(leftp).first);

    ASSERT_EQ(leftp.vertexCount(), leftControl.vertexCount());

    for(size_t i = 0; i < leftControl.vertexCount(); i++) {
        ASSERT_EQ(getX(leftp.vertex(i)), getX(leftControl.vertex(i)));
        ASSERT_EQ(getY(leftp.vertex(i)), getY(leftControl.vertex(i)));
    }

    downp = placer.downPoly(item);

    ASSERT_TRUE(ShapeLike::isValid(downp).first);

    ASSERT_EQ(downp.vertexCount(), downControl.vertexCount());

    for(size_t i = 0; i < downControl.vertexCount(); i++) {
        ASSERT_EQ(getX(downp.vertex(i)), getX(downControl.vertex(i)));
        ASSERT_EQ(getY(downp.vertex(i)), getY(downControl.vertex(i)));
    }
}

// Simple test, does not use gmock
TEST(GeometryAlgorithms, ArrangeRectanglesTight)
{
    using namespace binpack2d;

    std::vector<Rectangle> rects = {
        {80, 80},
        {60, 90},
        {70, 30},
        {80, 60},
        {60, 60},
        {60, 40},
        {40, 40},
        {10, 10},
        {10, 10},
        {10, 10},
        {10, 10},
        {10, 10},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {20, 20} };

    // Old MSVC2013 fucker does not recognize initializer list for structs
    BottomLeftPlacer::Config config;
    config.min_obj_distance = 0;

    Arranger arrange(Box(210, 250), config /*{.min_obj_distance = 10}*/ );

    auto groups = arrange(rects.begin(), rects.end());

    ASSERT_EQ(groups.size(), 1);
    ASSERT_EQ(groups[0].size(), rects.size());

    // check for no intersections, no containment:

    for(auto result : groups) {
        bool valid = true;
        for(Item& r1 : result) {
            for(Item& r2 : result) {
                if(&r1 != &r2 ) {
                    valid = !Item::intersects(r1, r2) || Item::touches(r1, r2);
                    valid = (valid && !r1.isInside(r2) && !r2.isInside(r1));
                    ASSERT_TRUE(valid);
                }
            }
        }
    }

}

TEST(GeometryAlgorithms, ArrangeRectanglesLoose)
{
    using namespace binpack2d;

//    std::vector<Rectangle> rects = { {40, 40}, {10, 10}, {20, 20} };
    std::vector<Rectangle> rects = {
        {80, 80},
        {60, 90},
        {70, 30},
        {80, 60},
        {60, 60},
        {60, 40},
        {40, 40},
        {10, 10},
        {10, 10},
        {10, 10},
        {10, 10},
        {10, 10},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {5, 5},
        {20, 20} };

    // Old MSVC2013 fucker does not recognize initializer list for structs
    BottomLeftPlacer::Config config;
    config.min_obj_distance = 5;

    Arranger arrange(Box(210, 250), config /*{.min_obj_distance = 10}*/ );

    auto groups = arrange(rects.begin(), rects.end());

    ASSERT_EQ(groups.size(), 1);
    ASSERT_EQ(groups[0].size(), rects.size());

    // check for no intersections, no containment:
    auto result = groups[0];
    bool valid = true;
    for(Item& r1 : result) {
        for(Item& r2 : result) {
            if(&r1 != &r2 ) {
                valid = !Item::intersects(r1, r2);
                valid = (valid && !r1.isInside(r2) && !r2.isInside(r1));
                ASSERT_TRUE(valid);
            }
        }
    }

}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
