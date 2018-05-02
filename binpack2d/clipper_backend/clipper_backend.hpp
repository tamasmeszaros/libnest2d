#ifndef CLIPPER_BACKEND_HPP
#define CLIPPER_BACKEND_HPP

#include <sstream>
#include <unordered_map>

#include "../binpack2d.hpp"

#include <clipper.hpp>

namespace binpack2d {

// Aliases for convinience
using PointImpl = ClipperLib::IntPoint;
using PolygonImpl = ClipperLib::PolyNode;
using PathImpl = ClipperLib::Path;

inline PointImpl& operator +=(PointImpl& p, const PointImpl& pa ) {
    p.X += pa.X;
    p.Y += pa.Y;
    return p;
}

inline PointImpl operator+(const PointImpl& p1, const PointImpl& p2) {
    PointImpl ret = p1;
    ret += p2;
    return ret;
}

inline PointImpl& operator -=(PointImpl& p, const PointImpl& pa ) {
    p.X -= pa.X;
    p.Y -= pa.Y;
    return p;
}

inline PointImpl operator-(const PointImpl& p1, const PointImpl& p2) {
    PointImpl ret = p1;
    ret -= p2;
    return ret;
}

class HoleCache {
    friend class ShapeLike;
    std::unordered_map< const PolygonImpl*, ClipperLib::Paths> map;

    ClipperLib::Paths& _getHoles(const PolygonImpl* p) {
        ClipperLib::Paths& paths = map[p];

        if(paths.size() != p->Childs.size()) {
            paths.reserve(p->Childs.size());

            for(auto np : p->Childs) {
                paths.emplace_back(np->Contour);
            }
        }

        return paths;
    }

    ClipperLib::Paths& getHoles(PolygonImpl& p) {
        return _getHoles(&p);
    }

    const ClipperLib::Paths& getHoles(const PolygonImpl& p) {
        return _getHoles(&p);
    }
};

extern HoleCache holeCache;

// Type of coordinate units used by Clipper
template<> struct CoordType<PointImpl> {
    using Type = ClipperLib::cInt;
};

// Type of point used by Clipper
template<> struct PointType<PolygonImpl> {
    using Type = PointImpl;
};

// Type of vertex iterator used by Clipper
template<> struct VertexIteratorType<PolygonImpl> {
    using Type = ClipperLib::Path::iterator;
};

// Type of vertex iterator used by Clipper
template<> struct VertexConstIteratorType<PolygonImpl> {
    using Type = ClipperLib::Path::const_iterator;
};

template<> struct CountourType<PolygonImpl> {
    using Type = PathImpl;
};


// Tell binpack2d how to extract the X coord from a ClipperPoint object
template<>
inline TCoord<PointImpl> PointLike::x(const PointImpl& p) {
    return p.X;
}

// Tell binpack2d how to extract the Y coord from a ClipperPoint object
template<>
inline TCoord<PointImpl> PointLike::y(const PointImpl& p) {
    return p.Y;
}

// Tell binpack2d how to extract the X coord from a ClipperPoint object
template<>
inline TCoord<PointImpl>& PointLike::x(PointImpl& p) {
    return p.X;
}

// Tell binpack2d how to extract the Y coord from a ClipperPoint object
template<>
inline TCoord<PointImpl>& PointLike::y(PointImpl& p) {
    return p.Y;
}


template<>
inline void ShapeLike::reserve(PolygonImpl& sh, unsigned long vertex_capacity) {
    return sh.Contour.reserve(vertex_capacity);
}

// Tell binpack2d how to make string out of a ClipperPolygon object
//template<>
//double ShapeLike::area(const PolygonImpl& sh) {
//    return abs(ClipperLib::Area(sh.Contour));
//}

// Tell binpack2d how to make string out of a ClipperPolygon object
template<> std::string ShapeLike::toString(const PolygonImpl& sh);

template<>
inline TVertexIterator<PolygonImpl> ShapeLike::begin(PolygonImpl& sh) {
    return sh.Contour.begin();
}

template<>
inline TVertexIterator<PolygonImpl> ShapeLike::end(PolygonImpl& sh) {
    return sh.Contour.end();
}

template<>
inline TVertexConstIterator<PolygonImpl> ShapeLike::cbegin(
        const PolygonImpl& sh)
{
    return sh.Contour.cbegin();
}

template<>
inline TVertexConstIterator<PolygonImpl> ShapeLike::cend(
        const PolygonImpl& sh)
{
    return sh.Contour.cend();
}

template<> struct HolesContainer<PolygonImpl> {
    using Type = ClipperLib::Paths;
};

template<>
PolygonImpl ShapeLike::create( std::initializer_list< PointImpl > il);

template<>
inline const THolesContainer<PolygonImpl>& ShapeLike::holes(
        const PolygonImpl& sh)
{
    return holeCache.getHoles(sh);
}

template<>
inline THolesContainer<PolygonImpl>& ShapeLike::holes(PolygonImpl& sh) {
    return holeCache.getHoles(sh);
}

template<>
inline TCountour<PolygonImpl>& ShapeLike::getHole(PolygonImpl& sh,
                                                  unsigned long idx)
{
    return sh.Childs[idx]->Contour;
}

template<>
inline const TCountour<PolygonImpl>& ShapeLike::getHole(const PolygonImpl& sh,
                                                        unsigned long idx) {
    return sh.Childs[idx]->Contour;
}

template<>
inline size_t ShapeLike::holeCount(const PolygonImpl& sh) {
    return sh.Childs.size();
}

template<>
inline PathImpl& ShapeLike::getContour(PolygonImpl& sh) {
    return sh.Contour;
}

template<>
inline const PathImpl& ShapeLike::getContour(const PolygonImpl& sh) {
    return sh.Contour;
}

}

//#define DISABLE_BOOST_SERIALIZE
//#define DISABLE_BOOST_UNSERIALIZE

// All other operators and algorithms are implemented with boost
#include "../boost_alg.hpp"

#endif // CLIPPER_BACKEND_HPP
