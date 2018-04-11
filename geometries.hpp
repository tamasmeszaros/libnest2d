#ifndef GEOMETRIES_H
#define GEOMETRIES_H

#include <memory>
#include <string>

namespace binpack2d {

using Unit = long;
class RawPoint;

class Point {
    Unit x_, y_;

public:

    Point(RawPoint&);

    operator RawPoint();

    Point(Unit x = Unit(), Unit y = Unit()): x_(x), y_(y) {}

    Unit x() const /*noexcept*/ { return x_; }

    Unit y() const /*noexcept*/ { return y_; }

    void x( Unit val ) /*noexcept*/ { x_ = val; }

    void y( Unit val ) /*noexcept*/ { y_ = val; }

};

class RawShape;
using RawShapePtr = std::unique_ptr<RawShape>;

class Shape {
    RawShapePtr rshape_;
public:
    Shape();
    ~Shape();

    Shape(const RawShape& rsh);

    Shape(const Shape& other);
    Shape& operator=(const Shape& );

    std::string toString();

protected:
    RawShape& rawShape() const { return *rshape_; }
};

class Rectangle: public Shape {
public:

    Rectangle(Unit width, Unit height);

    double area() const;

};

class Ellipse: public Shape {
public:
    using Shape::Shape;

};

class Polygon: public Shape {
public:

};

}

#endif // GEOMETRIES_H
