#ifndef BOOST_ALG_HPP
#define BOOST_ALG_HPP

#include <boost/geometry.hpp>

namespace boost {

using binpack2d::TCoord;
using binpack2d::PointImpl;
using binpack2d::PolygonImpl;

namespace geometry {
namespace traits {


/* ************************************************************************** */
/* Getting Boost know about aour point implementation *********************** */
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

// Boost would refer to ClipperLib::Path (alias PolygonImpl) as a multipolygon
template<> struct tag<PolygonImpl> {
    using type = ring_tag;
};

template<> struct point_order<PolygonImpl> {
    static const order_selector value = clockwise;
};

template<> struct closure<PolygonImpl> {
    static const closure_selector value = closed;
};


}   // traits
}   // geometry

template<>
struct range_value<PolygonImpl> {
    using type = PointImpl;
};

//template<>
//struct range_iterator<PolygonImpl> {
//    using type = PolygonImpl::iterator;
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
bool ShapeLike::intersects<PolygonImpl>(const PolygonImpl& sh1,
                                        const PolygonImpl& sh2) {
    return boost::geometry::intersects(sh1, sh2);
}

template<>
static bool ShapeLike::isInside<PolygonImpl>(const PointImpl& point,
                                             const PolygonImpl& shape) {

    std::string message;
    boost::geometry::is_valid(shape, message); // boost::geometry::within(point, shape);

    std::cout << message << std::endl;


    return boost::geometry::within(point, shape);
}

}



#endif // BOOST_ALG_HPP
