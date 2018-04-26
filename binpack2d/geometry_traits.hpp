#ifndef GEOMETRY_TRAITS_HPP
#define GEOMETRY_TRAITS_HPP

#include <string>
#include <type_traits>
#include <array>
#include <vector>

#include "common.hpp"

namespace binpack2d {

/// Getting the coordinate data type for a geometry class.
template<class GeomClass> struct CoordType { using Type = long; };

/// TCoord<GeomType> as shorthand for typename `CoordType<GeomType>::Type`.
template<class GeomType> using TCoord = typename CoordType<GeomType>::Type;

/// Getting the type of point structure used by a shape.
template<class Shape> struct PointType { using Type = void; };

/// TPoint<ShapeClass> as shorthand for `typename PointType<ShapeClass>::Type`.
template<class Shape> using TPoint = typename PointType<Shape>::Type;

/// Getting the VertexIterator type of a shape class.
template<class Shape> struct VertexIteratorType { using Type = void; };

/// Getting the const vertex iterator for a shape class.
template<class Shape> struct VertexConstIteratorType { using Type = void; };

/**
 * TVertexIterator<Shape> as shorthand for
 * `typename VertexIteratorType<Shape>::Type`
 */
template<class Shape>
using TVertexIterator = typename VertexIteratorType<Shape>::Type;

/**
 * \brief TVertexConstIterator<Shape> as shorthand for
 * `typename VertexConstIteratorType<Shape>::Type`
 */
template<class ShapeClass>
using TVertexConstIterator = typename VertexConstIteratorType<ShapeClass>::Type;

/**
 * \brief A point pair base class for other point pairs (segment, box, ...).
 * \tparam RawPoint The actual point type to use.
 */
template<class RawPoint>
struct PointPair {
    RawPoint p1;
    RawPoint p2;
};

/**
 * \brief An abstraction of a box;
 */
template<class RawPoint>
class _Box: PointPair<RawPoint> {
    using PointPair<RawPoint>::p1;
    using PointPair<RawPoint>::p2;
public:

    inline _Box() {}
    inline _Box(const RawPoint& p, const RawPoint& pp):
        PointPair<RawPoint>({p, pp}) {}

    inline _Box(TCoord<RawPoint> width, TCoord<RawPoint> height):
        _Box(RawPoint{0, 0}, RawPoint{width, height}) {}

    inline const RawPoint& minCorner() const BP2D_NOEXCEPT { return p1; }
    inline const RawPoint& maxCorner() const BP2D_NOEXCEPT { return p2; }

    inline RawPoint& minCorner() BP2D_NOEXCEPT { return p1; }
    inline RawPoint& maxCorner() BP2D_NOEXCEPT { return p2; }

    inline TCoord<RawPoint> width() const BP2D_NOEXCEPT;
    inline TCoord<RawPoint> height() const BP2D_NOEXCEPT;
};

template<class RawPoint>
class _Segment: PointPair<RawPoint> {
    using PointPair<RawPoint>::p1;
    using PointPair<RawPoint>::p2;
public:

    inline _Segment() {}
    inline _Segment(const RawPoint& p, const RawPoint& pp):
        PointPair<RawPoint>({p, pp}) {}

    inline const RawPoint& first() const BP2D_NOEXCEPT { return p1; }
    inline const RawPoint& second() const BP2D_NOEXCEPT { return p2; }

    inline RawPoint& first() BP2D_NOEXCEPT { return p1; }
    inline RawPoint& second() BP2D_NOEXCEPT { return p2; }
};

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
    static double distance(const RawPoint& /*p1*/, const RawPoint& /*p2*/) {
        throw UnimplementedException("PointLike::distance(point, point)");
    }

    template<class RawPoint>
    static double distance(const RawPoint& /*p1*/,
                           const _Segment<RawPoint>& /*s*/) {
        throw UnimplementedException("PointLike::distance(point, segment)");
    }

    template<class RawPoint>
    static std::pair<TCoord<RawPoint>, bool> horizontalDistance(
            const RawPoint& p, const _Segment<RawPoint>& s)
    {
        auto x = PointLike::x(p), y = PointLike::y(p);
        auto x1 = PointLike::x(s.first()), y1 = PointLike::y(s.first());
        auto x2 = PointLike::x(s.second()), y2 = PointLike::y(s.second());

        TCoord<RawPoint> ret;

        if( (y < y1 && y < y2) || (y > y1 && y > y2) )
            return {0, false};
        else if ((y == y1 && y == y2) && (x > x1 && x > x2))
            ret = std::min( x-x1, x -x2);
        else if( (y == y1 && y == y2) && (x < x1 && x < x2))
            ret = -std::min(x1 - x, x2 - x);
        else if(y == y1 && y == y2) // Problem if the coords are floating point!
            ret = 0;
        else
            ret = x - x1 + (x1 - x2)*(y1 - y)/(y1 - y2);

        return {ret, true};
    }

    template<class RawPoint>
    static std::pair<TCoord<RawPoint>, bool> verticalDistance(
            const RawPoint& p, const _Segment<RawPoint>& s)
    {
        auto x = PointLike::x(p), y = PointLike::y(p);
        auto x1 = PointLike::x(s.first()), y1 = PointLike::y(s.first());
        auto x2 = PointLike::x(s.second()), y2 = PointLike::y(s.second());

        TCoord<RawPoint> ret;

        if( (x < x1 && x < x2) || (x > x1 && x > x2) )
            return {0, false};
        else if ((x == x1 && x == x2) && (y > y1 && y > y2))
            ret = std::min( y-y1, y -y2);
        else if( (x == x1 && x == x2) && (y < y1 && y < y2))
            ret = -std::min(y1 - y, y2 - y);
        else if(x == x1 && x == x2) // Problem if the coords are floating point!
            ret = 0;
        else
            ret = y - y1 + (y1 - y2)*(x1 - x)/(x1 - x2);

        return {ret, true};
    }
};

template<class RawPoint>
TCoord<RawPoint> _Box<RawPoint>::width() const BP2D_NOEXCEPT {
    return PointLike::x(maxCorner()) - PointLike::x(minCorner());
}


template<class RawPoint>
TCoord<RawPoint> _Box<RawPoint>::height() const BP2D_NOEXCEPT {
    return PointLike::y(maxCorner()) - PointLike::y(minCorner());
}

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

template<class RawShape>
struct HolesContainer {
    using Type = std::vector<RawShape>;
};

template<class RawShape>
using THolesContainer = typename HolesContainer<RawShape>::Type;

template<class RawShape>
struct CountourType {
    using Type = RawShape;
};

template<class RawShape>
using TCountour = typename CountourType<RawShape>::Type;

enum class Orientation {
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

template<class RawShape>
struct OrientationType {

    // Default Polygon orientation that the library expects
    static const Orientation Value = Orientation::CLOCKWISE;
};

enum class Formats {
    WKT,
    SVG
};

class ShapeLike {
public:

    template<class RawShape>
    static RawShape create( std::initializer_list< TPoint<RawShape> > il)
    {
        return RawShape(il);
    }

    // Optional, does nothing by default
    template<class RawShape>
    static void reserve(RawShape& /*sh*/,  unsigned long /*vertex_capacity*/) {}

    template<class RawShape, class...Args>
    static void addVertex(RawShape& sh, Args...args) {
        return getContour(sh).emplace_back(std::forward<Args>(args)...);
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
    static TPoint<RawShape>& vertex(RawShape& sh, unsigned long idx) {
        return *(begin(sh) + idx);
    }

    template<class RawShape>
    static const TPoint<RawShape>& vertex(const RawShape& sh,
                                          unsigned long idx) {
        return *(cbegin(sh) + idx);
    }

    template<class RawShape>
    static size_t contourVertexCount(const RawShape& sh) {
        return cend(sh) - cbegin(sh);
    }

    template<class RawShape>
    static std::string toString(const RawShape& /*sh*/) {
        return "";
    }

    template<Formats, class RawShape>
    static std::string serialize(const RawShape& /*sh*/) {
        throw UnimplementedException("ShapeLike::serialize()");
    }

    template<Formats, class RawShape>
    static void unserialize(RawShape& /*sh*/, const std::string& /*str*/) {
        throw UnimplementedException("ShapeLike::unserialize()");
    }

    template<class RawShape>
    static double area(const RawShape& /*sh*/) {
        throw UnimplementedException("ShapeLike::area()");
        return 0;
    }

    template<class RawShape>
    static bool intersects(const RawShape& /*sh*/, const RawShape& /*sh*/) {
        throw UnimplementedException("ShapeLike::intersects()");
        return false;
    }

    template<class RawShape>
    static bool isInside(const TPoint<RawShape>& /*point*/,
                         const RawShape& /*shape*/) {
        throw UnimplementedException("ShapeLike::isInside(point, shape)");
        return false;
    }

    template<class RawShape>
    static bool isInside(const RawShape& /*shape*/,
                         const RawShape& /*shape*/) {
        throw UnimplementedException("ShapeLike::isInside(shape, shape)");
        return false;
    }

    template<class RawShape>
    static bool touches( const RawShape& /*shape*/,
                         const RawShape& /*shape*/) {
        return false;
    }

    template<class RawShape>
    static _Box<TPoint<RawShape>> boundingBox(const RawShape& /*sh*/) {
        throw UnimplementedException("ShapeLike::boundingBox(shape)");
        return _Box<TPoint<RawShape>>();
    }

    template<class RawShape>
    static THolesContainer<RawShape>& holes(RawShape& /*sh*/) {
        static THolesContainer<RawShape> empty;
        return empty;
    }

    template<class RawShape>
    static const THolesContainer<RawShape>& holes(const RawShape& /*sh*/) {
        static THolesContainer<RawShape> empty;
        return empty;
    }

    template<class RawShape>
    static TCountour<RawShape>& getHole(RawShape& sh, unsigned long idx) {
        return holes(sh)[idx];
    }

    template<class RawShape>
    static const TCountour<RawShape>& getHole(const RawShape& sh,
                                              unsigned long idx) {
        return holes(sh)[idx];
    }

    template<class RawShape>
    static size_t holeCount(const RawShape& sh) {
        return holes(sh).size();
    }

    template<class RawShape>
    static TCountour<RawShape>& getContour(RawShape& sh) {
        return sh;
    }

    template<class RawShape>
    static const TCountour<RawShape>& getContour(const RawShape& sh) {
        return sh;
    }

    template<class RawShape>
    static void rotate(RawShape& /*sh*/, const Radians& /*rads*/) {
        throw UnimplementedException("ShapeLike::rotate()");
    }

    template<class RawShape, class RawPoint>
    static void translate(RawShape& /*sh*/, const RawPoint& /*offs*/) {
        throw UnimplementedException("ShapeLike::translate()");
    }

    template<class RawShape>
    static std::pair<bool, std::string> isValid(const RawShape& sh) {
        return {true ,""};
    }

};

}

#endif // GEOMETRY_TRAITS_HPP
