#ifndef GEOMETRIES_NOFITPOLYGON_HPP
#define GEOMETRIES_NOFITPOLYGON_HPP

#include "geometry_traits.hpp"
#include <algorithm>

namespace libnest2d {

struct Nfp {

template<class RawShape>
static RawShape& minkowskiAdd(RawShape& sh, const RawShape& /*other*/) {
    static_assert(always_false<RawShape>::value,
                  "Nfp::minkowskiAdd() unimplemented!");
    return sh;
}

template<class RawShape>
static RawShape merge(const RawShape& sh1, const RawShape& sh2) {
    static_assert(always_false<RawShape>::value,
                  "Nfp::merge() unimplemented!");
}

template<class RawShape>
static TPoint<RawShape> referenceVertex(const RawShape& sh) {
    using Vertex = TPoint<RawShape>;
    using Coord = TCoord<Vertex>;

    // find min x and min y vertex
    auto it = std::min_element(ShapeLike::cbegin(sh), ShapeLike::cend(sh),
                               [](const Vertex& v1,
                                  const Vertex& v2)
    {
        auto diff = getX(v1) - getX(v2);
        if(std::abs(diff) <= std::numeric_limits<Coord>::epsilon())
            return getY(v1) < getY(v2);

        return diff < 0;
    });

    return *it;
}

template<class RawShape>
static RawShape noFitPolygon(const RawShape& sh, const RawShape& other) {
    auto isConvex = [](const RawShape& sh) {

        return true;
    };

    using Vertex = TPoint<RawShape>;
    using Edge = _Segment<Vertex>;

    auto nfpConvexConvex = [] (
            const RawShape& sh,
            const RawShape& cother)
    {
        RawShape other = cother;

        // Make the other polygon counter-clockwise
        for(auto shit = ShapeLike::begin(other);
            shit != ShapeLike::end(other); ++shit ) {
            auto& v = *shit;
            setX(v, -getX(v));
            setY(v, -getY(v));
        }

        RawShape rsh;   // Final nfp placeholder
        std::vector<Edge> edgelist;

        size_t cap = ShapeLike::contourVertexCount(sh) +
                ShapeLike::contourVertexCount(other);

        // Reserve the needed memory
        edgelist.reserve(cap);
        ShapeLike::reserve(rsh, cap);

        { // place all edges from sh into edgelist
            auto first = ShapeLike::cbegin(sh);
            auto next = first + 1;
            auto endit = ShapeLike::cend(sh);

            while(next != endit) edgelist.emplace_back(*(first++), *(next++));
        }

        { // place all edges from other into edgelist
            auto first = ShapeLike::cbegin(other);
            auto next = first + 1;
            auto endit = ShapeLike::cend(other);

            while(next != endit) edgelist.emplace_back(*(first++), *(next++));
        }

        // Sort the edges by angle to X axis.
        std::sort(edgelist.begin(), edgelist.end(),
                  [](const Edge& e1, const Edge& e2)
        {
            return e1.angleToXaxis() > e2.angleToXaxis();
        });

        // Add the two vertices from the first edge into the final polygon.
        ShapeLike::addVertex(rsh, edgelist.front().first());
        ShapeLike::addVertex(rsh, edgelist.front().second());

        auto tmp = std::next(ShapeLike::begin(rsh));

        // Construct final nfp by placing each edge to the end of the previous
        for(auto eit = std::next(edgelist.begin());
            eit != edgelist.end();
            ++eit) {

            auto dx = getX(*tmp) - getX(eit->first());
            auto dy = getY(*tmp) - getY(eit->first());

            ShapeLike::addVertex(rsh, getX(eit->second())+dx,
                                      getY(eit->second())+dy );

            tmp = std::next(tmp);
        }

        return rsh;
    };

    RawShape rsh;

    enum e_dispatch {
        CONVEX_CONVEX,
        CONCAVE_CONVEX,
        CONVEX_CONCAVE,
        CONCAVE_CONCAVE
    };

    int sel = isConvex(sh) ? CONVEX_CONVEX : CONCAVE_CONVEX;
    sel += isConvex(other) ? CONVEX_CONVEX : CONVEX_CONCAVE;

    switch(sel) {
    case CONVEX_CONVEX:
        rsh = nfpConvexConvex(sh, other); break;
    case CONCAVE_CONVEX:
        break;
    case CONVEX_CONCAVE:
        break;
    case CONCAVE_CONCAVE:
        break;
    }

    return rsh;
}

};

}

#endif // GEOMETRIES_NOFITPOLYGON_HPP
