#ifndef CLIPPER_BACKEND_HPP
#define CLIPPER_BACKEND_HPP

#include <sstream>

#include <Binpack2D/binpack2d.hpp>

#include <clipper.hpp>

namespace binpack2d {

// Aliases for convinience
using PointImpl = ClipperLib::IntPoint;
using PolygonImpl = ClipperLib::PolyNode;

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
    using Type = ClipperLib::Path::iterator;
};

// Type of vertex iterator used by Clipper
template<> struct VertexConstIteratorTypeOf<PolygonImpl> {
    using Type = ClipperLib::Path::const_iterator;
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
    return abs(ClipperLib::Area(sh.Contour));
}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
std::string ShapeLike::toString<PolygonImpl>(const PolygonImpl& sh) {
    std::stringstream ss;

    for(auto p : sh.Contour) {
        ss << p.X << " " << p.Y << "\n";
    }

    return ss.str();
}

template<>
TVertexIterator<PolygonImpl> ShapeLike::begin(PolygonImpl& sh) {
    return sh.Contour.begin();
}

template<>
TVertexIterator<PolygonImpl> ShapeLike::end(PolygonImpl& sh) {
    return sh.Contour.end();
}

template<>
TVertexConstIterator<PolygonImpl> ShapeLike::cbegin(const PolygonImpl& sh) {
    return sh.Contour.cbegin();
}

template<>
TVertexConstIterator<PolygonImpl> ShapeLike::cend(const PolygonImpl& sh) {
    return sh.Contour.cend();
}

template<>
struct HolesContainer<PolygonImpl> {
    using Type = ClipperLib::PolyNodes;
};


template<>
PolygonImpl ShapeLike::create( std::initializer_list< PointImpl > il)
{
    PolygonImpl p;
    p.Contour = il;
    return p;
}

template<>
static THolesContainer<PolygonImpl>& ShapeLike::holes(PolygonImpl& sh) {
    return sh.Childs;
}

template<>
const THolesContainer<PolygonImpl>& ShapeLike::holes(const PolygonImpl& sh) {
    return sh.Childs;
}

}

// All other operators and algorithms is implemented with boost
//#include "../boost_alg.hpp"

#endif // CLIPPER_BACKEND_HPP
