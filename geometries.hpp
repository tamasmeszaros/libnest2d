#ifndef GEOMETRIES_H
#define GEOMETRIES_H

#include <string>
#include <vector>
#include <tuple>

#include "geometry_traits.hpp"

namespace binpack2d {

template<class RawShape>
class Shape {
    RawShape sh_;
public:

    Shape(const RawShape& sh): sh_(sh) {}

    Shape(RawShape&& sh): sh_(std::move(sh)) {}

    std::string toString() const { return ShapeLike::toString(sh_); }

    Shape noFitPolygon() const { return Shape(); }

};

template<class RawShape>
class Rectangle {
    RawShape sh_;
public:

    using Unit = typename TCoord<RawShape>;

    Rectangle(Unit width, Unit height) {
        sh_ = ShapeLike::create(
            {{0, 0}, {0, height}, {width, height}, {width, 0}}
        );
    }
};


}

#endif // GEOMETRIES_H
