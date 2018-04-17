#ifndef BOOST_ALG_HPP
#define BOOST_ALG_HPP

#include <boost/geometry.hpp>

namespace {

template<class PolyType>
struct _PolyWrapper {
    PolyType poly;

    using TInterior = binpack2d::THolesContainer<PolyType>;

    _PolyWrapper(const PolyType& p): poly(p) {}

    TInterior& holes() { return binpack2d::ShapeLike::holes(poly); }
    const TInterior& holes() const { return binpack2d::ShapeLike::holes(poly); }
};

using PolyWrapper = _PolyWrapper<binpack2d::PolygonImpl>;

}

namespace boost {

using binpack2d::TCoord;
using binpack2d::PointImpl;
using binpack2d::PolygonImpl;

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
template<> struct tag<PolygonImpl> {
    using type = ring_tag;
};

template<> struct point_order<PolygonImpl> {
    static const order_selector value = clockwise;
};

template<> struct closure<PolygonImpl> {
    static const closure_selector value = closed;
};


//template<> struct tag<PolyWrapper> {
//    using type = polygon_tag;
//};

//template<> struct exterior_ring<PolyWrapper> {
//    static inline PolygonImpl& get(PolyWrapper& pw) { return pw.poly; }
//    static inline PolygonImpl const& get(PolyWrapper const& pw) {
//        return pw.poly;
//    }
//};

//template<> struct ring_const_type<PolyWrapper> {
//    using type = const PolygonImpl;
//};

//template<> struct ring_mutable_type<PolyWrapper> {
//    using type = PolygonImpl;
//};

//template<>
//struct interior_rings<PolyWrapper> {

//    static inline PolyWrapper::TInterior& get(PolyWrapper& pw) {
//        return pw.holes();
//    }

//    static inline const PolyWrapper::TInterior& get(PolyWrapper const& pw) {
//        return pw.holes();
//    }
//};

//template<> struct interior_const_type<PolyWrapper> {
//   using type = const PolyWrapper::TInterior;
//};

//template<> struct interior_mutable_type<PolyWrapper> {
//   using type = PolyWrapper::TInterior;
//};

}   // traits



}   // geometry

template<>
struct range_value<PolygonImpl> {
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

//    PolyWrapper pw = static_cast<PolyWrapper>(shape);

//    return boost::geometry::within(point, shape);

    return false;
//    boost::geometry::traits::interior_const_type
}

}



#endif // BOOST_ALG_HPP
