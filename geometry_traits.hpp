#ifndef GEOMETRY_TRAITS_HPP
#define GEOMETRY_TRAITS_HPP

#include <string>
#include <type_traits>
#include <array>

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

template<class ShapeClass>
struct VertexIteratorTypeOf {
    using Type = void;
};

template<class ShapeClass>
struct VertexConstIteratorTypeOf {
    using Type = void;
};

template<class ShapeClass>
using TVertexIterator = typename VertexIteratorTypeOf<ShapeClass>::Type;

template<class ShapeClass>
using TVertexConstIterator = typename VertexConstIteratorTypeOf<ShapeClass>::Type;


template<class ShapeClass>
struct TransformationTypeOf {
    using Type = std::array< std::array< TCoord<ShapeClass>, 3> , 3>;
};

template<class ShapeClass>
using TTransformation = typename TransformationTypeOf<ShapeClass>;

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

    template<class RawPoint>
    static TCoord<RawPoint>& x(RawPoint& p) {
        return p.x();
    }

    template<class RawPoint>
    static TCoord<RawPoint>& y(RawPoint& p) {
        return p.y();
    }

    template<class RawPoint>
    static double distance(const RawPoint& p1, const RawPoint& p2) {
        TCoord<RawPoint>();
    }
};

template<class RawPoint>
TCoord<RawPoint> getX(const RawPoint& p) { return PointLike::x<RawPoint>(p); }

template<class RawPoint>
TCoord<RawPoint> getY(const RawPoint& p) { return PointLike::y<RawPoint>(p); }

template<class RawPoint>
void setX(RawPoint& p, const TCoord<RawPoint>& val) {
    PointLike::x<RawPoint>(p) = val;
}

template<class RawPoint>
void setY(RawPoint& p, const TCoord<RawPoint>& val) {
    PointLike::y<RawPoint>(p) = val;
}

class TransformationLike{
public:

    template<class RawTransf>
    static TCoord<RawTransf>& get(const RawTransf& tr,
                                 unsigned long row,
                                 unsigned long col ) {
        return tr[col][row];
    }

};

class ShapeLike {
public:

    template<class RawShape>
    static RawShape create( std::initializer_list< TPoint<RawShape> > il)
    {
        return RawShape(il);
    }

    template<class RawShape>
    static TVertexIterator<RawShape> begin(RawShape& sh) {
        return sh.begin();
    }

    template<class RawShape>
    static TVertexIterator<RawShape> end(RawShape& sh) {
        return sh.end();
    }

    template<class RawShape>
    static TVertexConstIterator<RawShape> cbegin(const RawShape& sh) {
        return sh.cbegin();
    }

    template<class RawShape>
    static TVertexConstIterator<RawShape> cend(const RawShape& sh) {
        return sh.cend();
    }

    template<class RawShape>
    static TPoint<RawShape>& point(RawShape& sh, unsigned long idx) {
        return *(begin(sh) + idx);
    }

    template<class RawShape>
    static std::string toString(const RawShape& /*sh*/) {
        return "";
    }

    template<class RawShape, class RawTransf>
    static RawShape& transform(RawShape& sh, const RawTransf& /*tr*/) {
        // auto vertex_it = ShapeLike::begin(sh);
        // implement ...
        static_assert(false, "unimplemented");
        return sh;
    }

    template<class RawShape>
    static double area(const RawShape& /*sh*/) {
        return 0;
    }

    template<class RawShape>
    static bool intersects(const RawShape& /*sh*/, const RawShape& /*sh*/) {
        return false;
    }

    template<class RawShape>
    static bool isInside(const TPoint<RawShape>& point, const RawShape& shape) {
        return false;
    }

};


}

#endif // GEOMETRY_TRAITS_HPP
