#ifndef CLIPPER_BACKEND_HPP
#define CLIPPER_BACKEND_HPP

#include <sstream>

#include <Binpack2D/binpack2d.hpp>

#include <clipper.hpp>

namespace binpack2d {

// Aliases for convinience
using PointImpl = ClipperLib::IntPoint;
using PolygonImpl = ClipperLib::Path;

// Type of coordinate units used by Clipper
template<> struct CoordType<PointImpl> {
    using Type = ClipperLib::cInt;
};

// Type of point used by Clipper
template<> struct PointType<PolygonImpl> {
    using Type = PointImpl;
};

// Type of vertex iterator used by Clipper
template<> struct VertexIteratorTypeOf<PolygonImpl> {
    using Type = PolygonImpl::iterator;
};

// Type of vertex iterator used by Clipper
template<> struct VertexConstIteratorTypeOf<PolygonImpl> {
    using Type = PolygonImpl::const_iterator;
};


// Tell binpack2d how to extract the X coord from a ClipperPoint object
template<>
TCoord<PointImpl> PointLike::x<PointImpl>(const PointImpl& p) {
    return p.X;
}

// Tell binpack2d how to extract the Y coord from a ClipperPoint object
template<>
TCoord<PointImpl> PointLike::y<PointImpl>(const PointImpl& p) {
    return p.Y;
}

// Tell binpack2d how to extract the X coord from a ClipperPoint object
template<>
TCoord<PointImpl>& PointLike::x<PointImpl>(PointImpl& p) {
    return p.X;
}

// Tell binpack2d how to extract the Y coord from a ClipperPoint object
template<>
TCoord<PointImpl>& PointLike::y<PointImpl>(PointImpl& p) {
    return p.Y;
}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
double ShapeLike::area<PolygonImpl>(const PolygonImpl& sh) {
    return abs(ClipperLib::Area(sh));
}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
std::string ShapeLike::toString<PolygonImpl>(const PolygonImpl& sh) {
    std::stringstream ss;

    for(auto p : sh) {
        ss << p.X << " " << p.Y << "\n";
    }

    return ss.str();
}

}

// All other operators and algorithms is implemented with boost
#include "../boost_alg.hpp"

#endif // CLIPPER_BACKEND_HPP
