#ifndef BOOST_ALG_HPP
#define BOOST_ALG_HPP

#include <boost/geometry.hpp>
#include "binpack2d.h"

namespace boost {

using binpack2d::TCoord;
using binpack2d::PointImpl;
using binpack2d::PolygonImpl;
using binpack2d::PathImpl;

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

/* Connversion between binpack2d::Orientation and order_selector ************ */

template<binpack2d::Orientation O>
struct ToBoostOrienation {};

template<>
struct ToBoostOrienation<binpack2d::Orientation::CLOCKWISE> {
    static const order_selector Value = clockwise;
};

template<>
struct ToBoostOrienation<binpack2d::Orientation::COUNTER_CLOCKWISE> {
    static const order_selector Value = counterclockwise;
};

static const binpack2d::Orientation RealOrientation =
        binpack2d::OrientationType<PolygonImpl>::Value;

/* ************************************************************************** */

template<> struct point_order<PathImpl> {
    static const order_selector value =
            ToBoostOrienation<RealOrientation>::Value;
};

// All our Paths should be closed for the bin packing application
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

template<>
struct range_value<PathImpl> {
    using type = PointImpl;
};

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
bool ShapeLike::intersects(const PathImpl& sh1,
                                        const PathImpl& sh2) {
    return boost::geometry::intersects(sh1, sh2);
}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<>
bool ShapeLike::intersects(const PolygonImpl& sh1,
                                        const PolygonImpl& sh2) {
    return boost::geometry::intersects(sh1, sh2);
}

template<>
double ShapeLike::area(const PolygonImpl& shape) {
    return boost::geometry::area(shape);
}

template<>
bool ShapeLike::isInside(const PointImpl& point,
                         const PolygonImpl& shape)
{

//    std::string message;
//    boost::geometry::is_valid(shape, message); // boost::geometry::within(point, shape);

//    std::cout << message << std::endl;

    return boost::geometry::within(point, shape);
}


template<>
bool ShapeLike::isInside(const PolygonImpl& sh1,
                         const PolygonImpl& sh2)
{
    return boost::geometry::within(sh1, sh2);
}

}



#endif // BOOST_ALG_HPP
