#ifndef GEOMETRY_TRAITS_HPP
#define GEOMETRY_TRAITS_HPP

#include <string>
#include <type_traits>
#include <algorithm>
#include <array>
#include <vector>
#include <numeric>
#include <limits>
#include <iterator>
#include <cmath>

#include "common.hpp"

namespace libnest2d {

/// Getting the coordinate data type for a geometry class.
template<class GeomClass> struct CoordType { using Type = long; };

/// TCoord<GeomType> as shorthand for typename `CoordType<GeomType>::Type`.
template<class GeomType>
using TCoord = typename CoordType<remove_cvref_t<GeomType>>::Type;


/// Getting the type of point structure used by a shape.
template<class Sh> struct PointType { using Type = typename Sh::PointType; };

/// TPoint<ShapeClass> as shorthand for `typename PointType<ShapeClass>::Type`.
template<class Shape>
using TPoint = typename PointType<remove_cvref_t<Shape>>::Type;


template<class RawShape> struct CountourType { using Type = RawShape; };

template<class RawShape>
using TContour = typename CountourType<remove_cvref_t<RawShape>>::Type;


template<class RawShape>
struct HolesContainer { using Type = std::vector<TContour<RawShape>>;  };

template<class RawShape>
using THolesContainer = typename HolesContainer<remove_cvref_t<RawShape>>::Type;


template<class RawShape>
struct LastPointIsFirst { static const bool Value = true; };

enum class Orientation {
    CLOCKWISE,
    COUNTER_CLOCKWISE
};

template<class RawShape>
struct OrientationType {

    // Default Polygon orientation that the library expects
    static const Orientation Value = Orientation::CLOCKWISE;
};

/**
 * \brief A point pair base class for other point pairs (segment, box, ...).
 * \tparam RawPoint The actual point type to use.
 */
template<class RawPoint>
struct PointPair {
    RawPoint p1;
    RawPoint p2;
};

struct PolygonTag {};
struct PathTag {};
struct MultiPolygonTag {};
struct BoxTag {};
struct CircleTag {};

template<class Shape> struct ShapeTag { using Type = typename Shape::Tag; };
template<class S> using Tag = typename ShapeTag<remove_cvref_t<S>>::Type;

template<class S> struct MultiShape { using Type = std::vector<S>; };
template<class S>
using TMultiShape =typename MultiShape<remove_cvref_t<S>>::Type;

/**
 * \brief An abstraction of a box;
 */
template<class RawPoint>
class _Box: PointPair<RawPoint> {
    using PointPair<RawPoint>::p1;
    using PointPair<RawPoint>::p2;
public:

    using Tag = BoxTag;
    using PointType = RawPoint;

    inline _Box() = default;
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

    inline RawPoint center() const BP2D_NOEXCEPT;

    inline double area() const BP2D_NOEXCEPT {
        return double(width()*height());
    }
};

template<class RawPoint>
class _Circle {
    RawPoint center_;
    double radius_ = 0;
public:

    using Tag = CircleTag;
    using PointType = RawPoint;

    _Circle() = default;

    _Circle(const RawPoint& center, double r): center_(center), radius_(r) {}

    inline const RawPoint& center() const BP2D_NOEXCEPT { return center_; }
    inline const void center(const RawPoint& c) { center_ = c; }

    inline double radius() const BP2D_NOEXCEPT { return radius_; }
    inline void radius(double r) { radius_ = r; }

    inline double area() const BP2D_NOEXCEPT {
        return 2.0*Pi*radius_*radius_;
    }
};

/**
 * \brief An abstraction of a directed line segment with two points.
 */
template<class RawPoint>
class _Segment: PointPair<RawPoint> {
    using PointPair<RawPoint>::p1;
    using PointPair<RawPoint>::p2;
    mutable Radians angletox_ = std::nan("");
public:

    using PointType = RawPoint;

    inline _Segment() = default;

    inline _Segment(const RawPoint& p, const RawPoint& pp):
        PointPair<RawPoint>({p, pp}) {}

    /**
     * @brief Get the first point.
     * @return Returns the starting point.
     */
    inline const RawPoint& first() const BP2D_NOEXCEPT { return p1; }

    /**
     * @brief The end point.
     * @return Returns the end point of the segment.
     */
    inline const RawPoint& second() const BP2D_NOEXCEPT { return p2; }

    inline void first(const RawPoint& p) BP2D_NOEXCEPT
    {
        angletox_ = std::nan(""); p1 = p;
    }

    inline void second(const RawPoint& p) BP2D_NOEXCEPT {
        angletox_ = std::nan(""); p2 = p;
    }

    /// Returns the angle measured to the X (horizontal) axis.
    inline Radians angleToXaxis() const;

    /// The length of the segment in the measure of the coordinate system.
    inline double length();
};

// This struct serves almost as a namespace. The only difference is that is can
// used in friend declarations.
namespace pointlike {

template<class RawPoint>
inline TCoord<RawPoint> x(const RawPoint& p)
{
    return p.x();
}

template<class RawPoint>
inline TCoord<RawPoint> y(const RawPoint& p)
{
    return p.y();
}

template<class RawPoint>
inline TCoord<RawPoint>& x(RawPoint& p)
{
    return p.x();
}

template<class RawPoint>
inline TCoord<RawPoint>& y(RawPoint& p)
{
    return p.y();
}

template<class RawPoint>
inline double distance(const RawPoint& /*p1*/, const RawPoint& /*p2*/)
{
    static_assert(always_false<RawPoint>::value,
                  "PointLike::distance(point, point) unimplemented!");
    return 0;
}

template<class RawPoint>
inline double distance(const RawPoint& /*p1*/,
                       const _Segment<RawPoint>& /*s*/)
{
    static_assert(always_false<RawPoint>::value,
                  "PointLike::distance(point, segment) unimplemented!");
    return 0;
}

template<class RawPoint>
inline std::pair<TCoord<RawPoint>, bool> horizontalDistance(
        const RawPoint& p, const _Segment<RawPoint>& s)
{
    using Unit = TCoord<RawPoint>;
    auto x = pointlike::x(p), y = pointlike::y(p);
    auto x1 = pointlike::x(s.first()), y1 = pointlike::y(s.first());
    auto x2 = pointlike::x(s.second()), y2 = pointlike::y(s.second());

    TCoord<RawPoint> ret;

    if( (y < y1 && y < y2) || (y > y1 && y > y2) )
        return {0, false};
    if ((y == y1 && y == y2) && (x > x1 && x > x2))
        ret = std::min( x-x1, x -x2);
    else if( (y == y1 && y == y2) && (x < x1 && x < x2))
        ret = -std::min(x1 - x, x2 - x);
    else if(std::abs(y - y1) <= std::numeric_limits<Unit>::epsilon() &&
            std::abs(y - y2) <= std::numeric_limits<Unit>::epsilon())
        ret = 0;
    else
        ret = x - x1 + (x1 - x2)*(y1 - y)/(y1 - y2);

    return {ret, true};
}

template<class RawPoint>
inline std::pair<TCoord<RawPoint>, bool> verticalDistance(
        const RawPoint& p, const _Segment<RawPoint>& s)
{
    using Unit = TCoord<RawPoint>;
    auto x = pointlike::x(p), y = pointlike::y(p);
    auto x1 = pointlike::x(s.first()), y1 = pointlike::y(s.first());
    auto x2 = pointlike::x(s.second()), y2 = pointlike::y(s.second());

    TCoord<RawPoint> ret;

    if( (x < x1 && x < x2) || (x > x1 && x > x2) )
        return {0, false};
    if ((x == x1 && x == x2) && (y > y1 && y > y2))
        ret = std::min( y-y1, y -y2);
    else if( (x == x1 && x == x2) && (y < y1 && y < y2))
        ret = -std::min(y1 - y, y2 - y);
    else if(std::abs(x - x1) <= std::numeric_limits<Unit>::epsilon() &&
            std::abs(x - x2) <= std::numeric_limits<Unit>::epsilon())
        ret = 0;
    else
        ret = y - y1 + (y1 - y2)*(x1 - x)/(x1 - x2);

    return {ret, true};
}
}

template<class RawPoint>
TCoord<RawPoint> _Box<RawPoint>::width() const BP2D_NOEXCEPT
{
    return pointlike::x(maxCorner()) - pointlike::x(minCorner());
}

template<class RawPoint>
TCoord<RawPoint> _Box<RawPoint>::height() const BP2D_NOEXCEPT
{
    return pointlike::y(maxCorner()) - pointlike::y(minCorner());
}

template<class RawPoint>
TCoord<RawPoint> getX(const RawPoint& p) { return pointlike::x<RawPoint>(p); }

template<class RawPoint>
TCoord<RawPoint> getY(const RawPoint& p) { return pointlike::y<RawPoint>(p); }

template<class RawPoint>
void setX(RawPoint& p, const TCoord<RawPoint>& val)
{
    pointlike::x<RawPoint>(p) = val;
}

template<class RawPoint>
void setY(RawPoint& p, const TCoord<RawPoint>& val)
{
    pointlike::y<RawPoint>(p) = val;
}

template<class RawPoint>
inline Radians _Segment<RawPoint>::angleToXaxis() const
{
    if(std::isnan(static_cast<double>(angletox_))) {
        TCoord<RawPoint> dx = getX(second()) - getX(first());
        TCoord<RawPoint> dy = getY(second()) - getY(first());

        double a = std::atan2(dy, dx);
        auto s = std::signbit(a);

        if(s) a += Pi_2;
        angletox_ = a;
    }
    return angletox_;
}

template<class RawPoint>
inline double _Segment<RawPoint>::length()
{
    return pointlike::distance(first(), second());
}

template<class RawPoint>
inline RawPoint _Box<RawPoint>::center() const BP2D_NOEXCEPT {
    auto& minc = minCorner();
    auto& maxc = maxCorner();

    using Coord = TCoord<RawPoint>;

    RawPoint ret =  { // No rounding here, we dont know if these are int coords
        static_cast<Coord>( (getX(minc) + getX(maxc))/2.0 ),
        static_cast<Coord>( (getY(minc) + getY(maxc))/2.0 )
    };

    return ret;
}

enum class Formats {
    WKT,
    SVG
};

// This struct serves as a namespace. The only difference is that it can be
// used in friend declarations and can be aliased at class scope.
namespace shapelike {

template<class RawShape>
using Shapes = TMultiShape<RawShape>;

template<class RawShape>
inline RawShape create(const TContour<RawShape>& contour,
                       const THolesContainer<RawShape>& holes)
{
    return RawShape(contour, holes);
}

template<class RawShape>
inline RawShape create(TContour<RawShape>&& contour,
                       THolesContainer<RawShape>&& holes)
{
    return RawShape(contour, holes);
}

template<class RawShape>
inline RawShape create(const TContour<RawShape>& contour)
{
    return create<RawShape>(contour, {});
}

template<class RawShape>
inline RawShape create(TContour<RawShape>&& contour)
{
    return create<RawShape>(contour, {});
}

template<class RawShape>
inline THolesContainer<RawShape>& holes(RawShape& /*sh*/)
{
    static THolesContainer<RawShape> empty;
    return empty;
}

template<class RawShape>
inline const THolesContainer<RawShape>& holes(const RawShape& /*sh*/)
{
    static THolesContainer<RawShape> empty;
    return empty;
}

template<class RawShape>
inline TContour<RawShape>& hole(RawShape& sh, unsigned long idx)
{
    return holes(sh)[idx];
}

template<class RawShape>
inline const TContour<RawShape>& hole(const RawShape& sh, unsigned long idx)
{
    return holes(sh)[idx];
}

template<class RawShape>
inline size_t holeCount(const RawShape& sh)
{
    return holes(sh).size();
}

template<class RawShape>
inline TContour<RawShape>& contour(RawShape& sh)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::contour() unimplemented!");
    return sh;
}

template<class RawShape>
inline const TContour<RawShape>& contour(const RawShape& sh)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::contour() unimplemented!");
    return sh;
}

// Optional, does nothing by default
template<class RawPath>
inline void reserve(RawPath& p, size_t vertex_capacity, const PathTag&)
{
    p.reserve(vertex_capacity);
}

template<class RawShape, class...Args>
inline void addVertex(RawShape& sh, const PathTag&, Args...args)
{
    return sh.emplace_back(std::forward<Args>(args)...);
}

template<class RawShape, class Fn>
inline void foreachVertex(RawShape& sh, Fn fn, const PathTag&) {
    std::for_each(sh.begin(), sh.end(), fn);
}

template<class RawShape>
inline typename RawShape::iterator begin(RawShape& sh, const PathTag&)
{
    return sh.begin();
}

template<class RawShape>
inline typename RawShape::iterator end(RawShape& sh, const PathTag&)
{
    return sh.end();
}

template<class RawShape>
inline typename RawShape::const_iterator
cbegin(const RawShape& sh, const PathTag&)
{
    return sh.cbegin();
}

template<class RawShape>
inline typename RawShape::const_iterator
cend(const RawShape& sh, const PathTag&)
{
    return sh.cend();
}

template<class RawShape>
inline std::string toString(const RawShape& /*sh*/)
{
    return "";
}

template<Formats, class RawShape>
inline std::string serialize(const RawShape& /*sh*/, double /*scale*/=1)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::serialize() unimplemented!");
    return "";
}

template<Formats, class RawShape>
inline void unserialize(RawShape& /*sh*/, const std::string& /*str*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::unserialize() unimplemented!");
}

template<class RawShape>
inline double area(const RawShape& /*sh*/, const PolygonTag&)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::area() unimplemented!");
    return 0;
}

template<class RawShape>
inline bool intersects(const RawShape& /*sh*/, const RawShape& /*sh*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::intersects() unimplemented!");
    return false;
}

template<class RawShape>
inline bool isInside(const TPoint<RawShape>& /*point*/,
                     const RawShape& /*shape*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::isInside(point, shape) unimplemented!");
    return false;
}

template<class RawShape>
inline bool isInside(const RawShape& /*shape*/,
                     const RawShape& /*shape*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::isInside(shape, shape) unimplemented!");
    return false;
}

template<class RawShape>
inline bool touches( const RawShape& /*shape*/,
                     const RawShape& /*shape*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::touches(shape, shape) unimplemented!");
    return false;
}

template<class RawShape>
inline bool touches( const TPoint<RawShape>& /*point*/,
                     const RawShape& /*shape*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::touches(point, shape) unimplemented!");
    return false;
}

template<class RawShape>
inline _Box<TPoint<RawShape>> boundingBox(const RawShape& /*sh*/,
                                          const PolygonTag&)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::boundingBox(shape) unimplemented!");
}

template<class RawShapes>
inline _Box<TPoint<typename RawShapes::value_type>>
boundingBox(const RawShapes& /*sh*/, const MultiPolygonTag&)
{
    static_assert(always_false<RawShapes>::value,
                  "shapelike::boundingBox(shapes) unimplemented!");
}

template<class RawShape>
inline RawShape convexHull(const RawShape& /*sh*/, const PolygonTag&)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::convexHull(shape) unimplemented!");
    return RawShape();
}

template<class RawShapes>
inline typename RawShapes::value_type
convexHull(const RawShapes& /*sh*/, const MultiPolygonTag&)
{
    static_assert(always_false<RawShapes>::value,
                  "shapelike::convexHull(shapes) unimplemented!");
    return typename RawShapes::value_type();
}

template<class RawShape>
inline void rotate(RawShape& /*sh*/, const Radians& /*rads*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::rotate() unimplemented!");
}

template<class RawShape, class RawPoint>
inline void translate(RawShape& /*sh*/, const RawPoint& /*offs*/)
{
    static_assert(always_false<RawShape>::value,
                  "shapelike::translate() unimplemented!");
}

template<class RawShape>
inline void offset(RawShape& /*sh*/, TCoord<TPoint<RawShape>> /*distance*/)
{
    dout() << "The current geometry backend does not support offsetting!\n";
}

template<class RawShape>
inline std::pair<bool, std::string> isValid(const RawShape& /*sh*/)
{
    return {false, "shapelike::isValid() unimplemented!"};
}

template<class RawPath> inline bool isConvex(const RawPath& sh, const PathTag&)
{
    using Vertex = TPoint<RawPath>;
    auto first = begin(sh);
    auto middle = std::next(first);
    auto last = std::next(middle);
    using CVrRef = const Vertex&;

    auto zcrossproduct = [](CVrRef k, CVrRef k1, CVrRef k2) {
        auto dx1 = getX(k1) - getX(k);
        auto dy1 = getY(k1) - getY(k);
        auto dx2 = getX(k2) - getX(k1);
        auto dy2 = getY(k2) - getY(k1);
        return dx1*dy2 - dy1*dx2;
    };

    auto firstprod = zcrossproduct( *(std::prev(std::prev(end(sh)))),
                                    *first,
                                    *middle );

    bool ret = true;
    bool frsign = firstprod > 0;
    while(last != end(sh)) {
        auto &k = *first, &k1 = *middle, &k2 = *last;
        auto zc = zcrossproduct(k, k1, k2);
        ret &= frsign == (zc > 0);
        ++first; ++middle; ++last;
    }

    return ret;
}

// *****************************************************************************
// No need to implement these
// *****************************************************************************

template<class RawShape>
inline typename TContour<RawShape>::iterator
begin(RawShape& sh, const PolygonTag& t)
{
    return begin(contour(sh), PathTag());
}

template<class RawShape> // Tag dispatcher
inline auto begin(RawShape& sh) -> decltype(begin(sh, Tag<RawShape>()))
{
    return begin(sh, Tag<RawShape>());
}

template<class RawShape>
inline typename TContour<RawShape>::const_iterator
cbegin(const RawShape& sh, const PolygonTag&)
{
    return cbegin(contour(sh), PathTag());
}

template<class RawShape> // Tag dispatcher
inline auto cbegin(const RawShape& sh) -> decltype(cbegin(sh, Tag<RawShape>()))
{
    return cbegin(sh, Tag<RawShape>());
}

template<class RawShape>
inline typename TContour<RawShape>::iterator
end(RawShape& sh, const PolygonTag&)
{
    return end(contour(sh), PathTag());
}

template<class RawShape> // Tag dispatcher
inline auto end(RawShape& sh) -> decltype(begin(sh, Tag<RawShape>()))
{
    return end(sh, Tag<RawShape>());
}

template<class RawShape>
inline typename TContour<RawShape>::const_iterator
cend(const RawShape& sh, const PolygonTag&)
{
    return cend(contour(sh), PathTag());
}

template<class RawShape> // Tag dispatcher
inline auto cend(const RawShape& sh) -> decltype(cend(sh, Tag<RawShape>()))
{
    return cend(sh, Tag<RawShape>());
}

template<class It> std::reverse_iterator<It> _backward(It iter) {
    return std::reverse_iterator<It>(iter);
}

template<class P> auto rbegin(P& p) -> decltype(_backward(end(p)))
{
    return _backward(end(p));
}

template<class P> auto rcbegin(const P& p) -> decltype(_backward(end(p)))
{
    return _backward(end(p));
}

template<class P> auto rend(P& p) -> decltype(_backward(begin(p)))
{
    return _backward(begin(p));
}

template<class P> auto rcend(const P& p) -> decltype(_backward(cbegin(p)))
{
    return _backward(cbegin(p));
}

template<class P> TPoint<P> front(const P& p) { return *shapelike::cbegin(p); }
template<class P> TPoint<P> back (const P& p) {
    return *backward(shapelike::cend(p));
}

// Optional, does nothing by default
template<class RawShape>
inline void reserve(RawShape& sh, size_t vertex_capacity, const PolygonTag&)
{
    reserve(contour(sh), vertex_capacity, PathTag());
}

template<class T> // Tag dispatcher
inline void reserve(T& sh, size_t vertex_capacity) {
    reserve(sh, vertex_capacity, Tag<T>());
}

template<class RawShape, class...Args>
inline void addVertex(RawShape& sh, const PolygonTag&, Args...args)
{
    return addVertex(contour(sh), PathTag(), std::forward<Args>(args)...);
}

template<class RawShape, class...Args> // Tag dispatcher
inline void addVertex(RawShape& sh, Args...args)
{
    return addVertex(sh, Tag<RawShape>(), std::forward<Args>(args)...);
}

template<class Box>
inline Box boundingBox(const Box& box, const BoxTag& )
{
    return box;
}

template<class Circle>
inline _Box<typename Circle::PointType> boundingBox(
        const Circle& circ, const CircleTag&)
{
    using Point = typename Circle::PointType;
    using Coord = TCoord<Point>;
    Point pmin = {
        static_cast<Coord>(getX(circ.center()) - circ.radius()),
        static_cast<Coord>(getY(circ.center()) - circ.radius()) };

    Point pmax = {
        static_cast<Coord>(getX(circ.center()) + circ.radius()),
        static_cast<Coord>(getY(circ.center()) + circ.radius()) };

    return {pmin, pmax};
}

template<class S> // Dispatch function
inline _Box<TPoint<S>> boundingBox(const S& sh)
{
    return boundingBox(sh, Tag<S>() );
}

template<class Box>
inline double area(const Box& box, const BoxTag& )
{
    return box.area();
}

template<class Circle>
inline double area(const Circle& circ, const CircleTag& )
{
    return circ.area();
}

template<class RawShape> // Dispatching function
inline double area(const RawShape& sh)
{
    return area(sh, Tag<RawShape>());
}

template<class RawShapes>
inline double area(const RawShapes& shapes, const MultiPolygonTag&)
{
    using RawShape = typename RawShapes::value_type;
    return std::accumulate(shapes.begin(), shapes.end(), 0.0,
                    [](double a, const RawShape& b) {
        return a += area(b);
    });
}

template<class RawShape>
inline auto convexHull(const RawShape& sh)
    -> decltype(convexHull(sh, Tag<RawShape>())) // TODO: C++14 could deduce
{
    return convexHull(sh, Tag<RawShape>());
}

template<class RawShape>
inline bool isInside(const TPoint<RawShape>& point,
                     const _Circle<TPoint<RawShape>>& circ)
{
    return pointlike::distance(point, circ.center()) < circ.radius();
}

template<class RawShape>
inline bool isInside(const TPoint<RawShape>& point,
                     const _Box<TPoint<RawShape>>& box)
{
    auto px = getX(point);
    auto py = getY(point);
    auto minx = getX(box.minCorner());
    auto miny = getY(box.minCorner());
    auto maxx = getX(box.maxCorner());
    auto maxy = getY(box.maxCorner());

    return px > minx && px < maxx && py > miny && py < maxy;
}

template<class RawShape>
inline bool isInside(const RawShape& sh,
                     const _Circle<TPoint<RawShape>>& circ)
{
    return std::all_of(cbegin(sh), cend(sh),
                       [&circ](const TPoint<RawShape>& p){
        return isInside<RawShape>(p, circ);
    });
}

template<class RawShape>
inline bool isInside(const _Box<TPoint<RawShape>>& box,
                     const _Circle<TPoint<RawShape>>& circ)
{
    return isInside<RawShape>(box.minCorner(), circ) &&
            isInside<RawShape>(box.maxCorner(), circ);
}

template<class RawShape>
inline bool isInside(const _Box<TPoint<RawShape>>& ibb,
                     const _Box<TPoint<RawShape>>& box)
{
    auto iminX = getX(ibb.minCorner());
    auto imaxX = getX(ibb.maxCorner());
    auto iminY = getY(ibb.minCorner());
    auto imaxY = getY(ibb.maxCorner());

    auto minX = getX(box.minCorner());
    auto maxX = getX(box.maxCorner());
    auto minY = getY(box.minCorner());
    auto maxY = getY(box.maxCorner());

    return iminX > minX && imaxX < maxX && iminY > minY && imaxY < maxY;
}

template<class RawShape> // Potential O(1) implementation may exist
inline TPoint<RawShape>& vertex(RawShape& sh, unsigned long idx,
                                const PolygonTag&)
{
    return *(shapelike::begin(contour(sh)) + idx);
}

template<class RawShape> // Potential O(1) implementation may exist
inline TPoint<RawShape>& vertex(RawShape& sh, unsigned long idx,
                                const PathTag&)
{
    return *(shapelike::begin(sh) + idx);
}

template<class RawShape> // Potential O(1) implementation may exist
inline TPoint<RawShape>& vertex(RawShape& sh, unsigned long idx)
{
    return vertex(sh, idx, Tag<RawShape>());
}

template<class RawShape> // Potential O(1) implementation may exist
inline const TPoint<RawShape>& vertex(const RawShape& sh,
                                      unsigned long idx,
                                      const PolygonTag&)
{
    return *(shapelike::cbegin(contour(sh)) + idx);
}

template<class RawShape> // Potential O(1) implementation may exist
inline const TPoint<RawShape>& vertex(const RawShape& sh,
                                      unsigned long idx,
                                      const PathTag&)
{
    return *(shapelike::cbegin(sh) + idx);
}


template<class RawShape> // Potential O(1) implementation may exist
inline const TPoint<RawShape>& vertex(const RawShape& sh,
                                      unsigned long idx)
{
    return vertex(sh, idx, Tag<RawShape>());
}

template<class RawShape>
inline size_t contourVertexCount(const RawShape& sh)
{
    return shapelike::cend(sh) - shapelike::cbegin(sh);
}

template<class RawShape, class Fn>
inline void foreachVertex(RawShape& sh, Fn fn, const PolygonTag&) {
    foreachVertex(contour(sh), fn, PathTag());
    for(auto& h : holes(sh)) foreachVertex(h, fn, PathTag());
}

template<class RawShape, class Fn>
inline void foreachVertex(RawShape& sh, Fn fn) {
    foreachVertex(sh, fn, Tag<RawShape>());
}

template<class Poly> inline bool isConvex(const Poly& sh, const PolygonTag&)
{
    bool convex = true;
    convex &= isConvex(contour(sh), PathTag());
    convex &= holeCount(sh) == 0;
    return convex;
}

template<class RawShape> inline bool isConvex(const RawShape& sh) // dispatch
{
    return isConvex(sh, Tag<RawShape>());
}

}

#define DECLARE_MAIN_TYPES(T)        \
    using Polygon = T;               \
    using Point   = TPoint<T>;       \
    using Coord   = TCoord<Point>;   \
    using Contour = TContour<T>;     \
    using Box     = _Box<Point>;     \
    using Circle  = _Circle<Point>;  \
    using Segment = _Segment<Point>; \
    using Polygons = TMultiShape<T>

}

#endif // GEOMETRY_TRAITS_HPP
