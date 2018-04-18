#ifndef BOOST_ALG_HPP
#define BOOST_ALG_HPP

#include <boost/geometry.hpp>
#include "binpack2d.h"

//namespace {

//template<class PolyType>
//struct _PolyWrapper {
//    PolyType poly;

//    using TInterior = binpack2d::THolesContainer<PolyType>;

//    _PolyWrapper(const PolyType& p): poly(p) {}

//    TInterior& holes() { return binpack2d::ShapeLike::holes(poly); }
//    const TInterior& holes() const { return binpack2d::ShapeLike::holes(poly); }
//};

//using PolyWrapper = _PolyWrapper<binpack2d::PolygonImpl>;

//}

namespace boost {

using binpack2d::TCoord;
using binpack2d::PointImpl;
using binpack2d::PolygonImpl;
using binpack2d::PathImpl;

//struct ConstPolyWrapper {
//    const PolygonImpl& poly;

//    using TInterior = binpack2d::HolesRange<PolygonImpl>::ContainerType;

//    TInterior holes;

//    ConstPolyWrapper(const PolygonImpl& p):
//        poly(p),
//        holes(binpack2d::ShapeLike::holes(p).first,
//              binpack2d::ShapeLike::holes(p).last) {}
//};


namespace geometry {
namespace traits {


/* ************************************************************************** */
/* Getting Boost know about our point implementation ************************ */
/* ************************************************************************** */

template<> struct tag<PointImpl> {
    using type = point_tag;
};

template<> struct coordinate_type<PointImpl> {
    using type = TCoord<PointImpl>;
};

template<> struct coordinate_system<PointImpl> {
    using type = cs::cartesian;
};

template<> struct dimension<PointImpl>: boost::mpl::int_<2> {};

template<>
struct access<PointImpl, 0 > {
    static inline TCoord<PointImpl> get(PointImpl const& a) {
        return binpack2d::getX(a);
    }

    static inline void set(PointImpl& a, TCoord<PointImpl> const& value) {
        binpack2d::setX(a, value);
    }
};

template<>
struct access<PointImpl, 1 > {
    static inline TCoord<PointImpl> get(PointImpl const& a) {
        return binpack2d::getY(a);
    }

    static inline void set(PointImpl& a, TCoord<PointImpl> const& value) {
        binpack2d::setY(a, value);
    }
};


/* ************************************************************************** */
/* Getting Boost know about our polygon implementation ********************** */
/* ************************************************************************** */


// Boost would refer to ClipperLib::Path (alias PolygonImpl) as a ring
template<> struct tag<PathImpl> {
    using type = ring_tag;
};

template<> struct point_order<PathImpl> {
    static const order_selector value = clockwise;
};

template<> struct closure<PathImpl> {
    static const closure_selector value = closed;
};

template<> struct tag<PolygonImpl> {
    using type = polygon_tag;
};

template<> struct exterior_ring<PolygonImpl> {
    static inline PathImpl& get(PolygonImpl& p) {
        return binpack2d::ShapeLike::getContour(p);
    }

    static inline PathImpl const& get(PolygonImpl const& p) {
        return binpack2d::ShapeLike::getContour(p);
    }
};

template<> struct ring_const_type<PolygonImpl> {
    using type = const PathImpl;
};

template<> struct ring_mutable_type<PolygonImpl> {
    using type = PathImpl;
};

template<> struct interior_const_type<PolygonImpl> {
   using type = const binpack2d::THolesContainer<PolygonImpl>;
};

template<> struct interior_mutable_type<PolygonImpl> {
   using type = binpack2d::THolesContainer<PolygonImpl>;
};

template<>
struct interior_rings<PolygonImpl> {

    static inline binpack2d::THolesContainer<PolygonImpl>& get(
            PolygonImpl& p) {
        return binpack2d::ShapeLike::holes(p);
    }

    static inline const binpack2d::THolesContainer<PolygonImpl>& get(
            PolygonImpl const& p) {
        return binpack2d::ShapeLike::holes(p);
    }
};

}   // traits
}   // geometry

//template<>
//struct range_value<PolygonImpl> {
//    using type = PointImpl;
//};

}   // boost

namespace binpack2d {

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
double PointLike::distance<PointImpl>(const PointImpl& p1,
                                      const PointImpl& p2 ) {
    return boost::geometry::distance(p1, p2);
}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
bool ShapeLike::intersects<PathImpl>(const PathImpl& sh1,
                                        const PathImpl& sh2) {
    return boost::geometry::intersects(sh1, sh2);
}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
bool ShapeLike::intersects<PolygonImpl>(const PolygonImpl& sh1,
                                        const PolygonImpl& sh2) {
    return boost::geometry::intersects(sh1, sh2);
}

template<>
static bool ShapeLike::isInside<PolygonImpl>(const PointImpl& point,
                                             const PolygonImpl& shape) {

//    std::string message;
//    boost::geometry::is_valid(shape, message); // boost::geometry::within(point, shape);

//    std::cout << message << std::endl;

    return boost::geometry::within(point, shape);
}

}



#endif // BOOST_ALG_HPP
