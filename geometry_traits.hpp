#ifndef GEOMETRY_TRAITS_HPP
#define GEOMETRY_TRAITS_HPP

#include <string>
#include <type_traits>

namespace binpack2d {

/**
 * Getting the coordinate data type for a geometry class
 */
template<class GeomClass>
struct CoordType {
    using Type = long;
};

/// TCoord<GeomType> is now a shorthand for CoordType<GeomType>::Type
template<class GeomType>
using TCoord = typename CoordType<GeomType>::Type;

template<class ShapeClass>
struct PointType {
    using Type = void;
};

template<class ShapeClass>
using TPoint = typename PointType<ShapeClass>::Type;

class PointLike {
public:

    template<class RawPoint>
    static TCoord<RawPoint> x(const RawPoint& p) {
        return p.x();
    }

    template<class RawPoint>
    static TCoord<RawPoint> y(const RawPoint& p) {
        return p.y();
    }

};

class ShapeLike {
public:

    template<class RawShape>
    static RawShape& rotate(RawShape& sh) { return sh.rotate(); }

    template<class RawShape>
    static RawShape create( std::initializer_list< TPoint<RawShape> > il)
    {
        return RawShape(il);
    }

    template<class RawShape>
    static RawShape noFitPolygon(const RawShape& sh) {
        return RawShape();      // we can't do no fit polygon, cos we're dumb
    }

    template<class RawShape>
    static std::string toString(const RawShape& /*sh*/) {
        return "";
    }

};

}

#endif // GEOMETRY_TRAITS_HPP
