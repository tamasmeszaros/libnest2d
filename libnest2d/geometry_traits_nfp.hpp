#ifndef GEOMETRIES_NOFITPOLYGON_HPP
#define GEOMETRIES_NOFITPOLYGON_HPP

#include "geometry_traits.hpp"
#include <algorithm>
#include <functional>
#include <vector>

namespace libnest2d {

/// The complexity level of a polygon that an NFP implementation can handle.
enum class NfpLevel: unsigned {
    CONVEX_ONLY,
    ONE_CONVEX,
    BOTH_CONCAVE,
    ONE_CONVEX_WITH_HOLES,
    BOTH_CONCAVE_WITH_HOLES
};

/// A collection of static methods for handling the no fit polygon creation.
struct Nfp {

// Shorthand for a pile of polygons
template<class RawShape>
using Shapes = typename ShapeLike::Shapes<RawShape>;

/**
 * Merge a bunch of polygons with the specified additional polygon.
 *
 * \tparam RawShape the Polygon data type.
 * \param shc The pile of polygons that will be unified with sh.
 * \param sh A single polygon to unify with shc.
 *
 * \return A set of polygons that is the union of the input polygons. Note that
 * mostly it will be a set containing only one big polygon but if the input
 * polygons are disjuct than the resulting set will contain more polygons.
 */
template<class RawShape>
static Shapes<RawShape> merge(const Shapes<RawShape>& /*shc*/,
                              const RawShape& /*sh*/)
{
    static_assert(always_false<RawShape>::value,
                  "Nfp::merge(shapes, shape) unimplemented!");
}

/**
 * A method to get a vertex from a polygon that always maintains a relative
 * position to the coordinate system: It is always the rightmost top vertex.
 *
 * This way it does not matter in what order the vertices are stored, the
 * reference will be always the same for the same polygon.
 */
template<class RawShape>
inline static TPoint<RawShape> referenceVertex(const RawShape& sh)
{
    return rightmostUpVertex(sh);
}

/**
 * Get the vertex of the polygon that is at the lowest values (bottom) in the Y
 * axis and if there are more than one vertices on the same Y coordinate than
 * the result will be the leftmost (with the highest X coordinate).
 */
template<class RawShape>
static TPoint<RawShape> leftmostDownVertex(const RawShape& sh)
{

    // find min x and min y vertex
    auto it = std::min_element(ShapeLike::cbegin(sh), ShapeLike::cend(sh),
                               _vsort<RawShape>);

    return *it;
}

/**
 * Get the vertex of the polygon that is at the highest values (top) in the Y
 * axis and if there are more than one vertices on the same Y coordinate than
 * the result will be the rightmost (with the lowest X coordinate).
 */
template<class RawShape>
static TPoint<RawShape> rightmostUpVertex(const RawShape& sh)
{

    // find max x and max y vertex
    auto it = std::max_element(ShapeLike::cbegin(sh), ShapeLike::cend(sh),
                               _vsort<RawShape>);

    return *it;
}

template<class RawShape>
using NfpResult = std::pair<RawShape, TPoint<RawShape>>;

/// Helper function to get the NFP
template<NfpLevel nfptype, class RawShape>
static NfpResult<RawShape> noFitPolygon(const RawShape& sh,
                                        const RawShape& other)
{
    NfpImpl<RawShape, nfptype> nfp;
    return nfp(sh, other);
}

/**
 * The "trivial" Cuninghame-Green implementation of NFP for convex polygons.
 *
 * You can use this even if you provide implementations for the more complex
 * cases (Through specializing the the NfpImpl struct). Currently, no other
 * cases are covered in the library.
 *
 * Complexity should be no more than linear in the number of edges of the input
 * polygons.
 *
 * \tparam RawShape the Polygon data type.
 * \param sh The stationary polygon
 * \param cother The orbiting polygon
 * \return Returns a pair of the NFP and its reference vertex of the two input
 * polygons which have to be strictly convex. The resulting NFP is proven to be
 * convex as well in this case.
 *
 */
template<class RawShape>
static NfpResult<RawShape> nfpConvexOnly(const RawShape& sh,
                                         const RawShape& cother)
{
    using Vertex = TPoint<RawShape>; using Edge = _Segment<Vertex>;
    using sl = ShapeLike;

    RawShape other = cother;

    // Make the other polygon counter-clockwise
    std::reverse(sl::begin(other), sl::end(other));

    RawShape rsh;   // Final nfp placeholder
    Vertex top_nfp;
    std::vector<Edge> edgelist;

    auto cap = sl::contourVertexCount(sh) + sl::contourVertexCount(other);

    // Reserve the needed memory
    edgelist.reserve(cap);
    sl::reserve(rsh, static_cast<unsigned long>(cap));

    { // place all edges from sh into edgelist
        auto first = sl::cbegin(sh);
        auto next = first + 1;
        auto endit = sl::cend(sh);

        while(next != endit) edgelist.emplace_back(*(first++), *(next++));
    }

    { // place all edges from other into edgelist
        auto first = sl::cbegin(other);
        auto next = first + 1;
        auto endit = sl::cend(other);

        while(next != endit) edgelist.emplace_back(*(first++), *(next++));
    }

    // Sort the edges by angle to X axis.
    std::sort(edgelist.begin(), edgelist.end(),
              [](const Edge& e1, const Edge& e2)
    {
        return e1.angleToXaxis() > e2.angleToXaxis();
    });

    // Add the two vertices from the first edge into the final polygon.
    sl::addVertex(rsh, edgelist.front().first());
    sl::addVertex(rsh, edgelist.front().second());

    // Sorting function for the nfp reference vertex search
    auto& cmp = _vsort<RawShape>;

    // the reference (rightmost top) vertex so far
    top_nfp = *std::max_element(sl::cbegin(rsh), sl::cend(rsh), cmp );

    auto tmp = std::next(sl::begin(rsh));

    // Construct final nfp by placing each edge to the end of the previous
    for(auto eit = std::next(edgelist.begin());
        eit != edgelist.end();
        ++eit)
    {
        auto d = *tmp - eit->first();
        Vertex p = eit->second() + d;

        sl::addVertex(rsh, p);

        // Set the new reference vertex
        if(cmp(top_nfp, p)) top_nfp = p;

        tmp = std::next(tmp);
    }

    return {rsh, top_nfp};
}

template<class RawShape>
static NfpResult<RawShape> nfpSimpleSimple(const RawShape& stationary,
                                           const RawShape& cother)
{
    using Result = NfpResult<RawShape>;
    using Vertex = TPoint<RawShape>;
    using Coord = TCoord<Vertex>;
    using Edge = _Segment<Vertex>;
    using sl = ShapeLike;
    using std::signbit;
    using std::sort;
    using std::vector;
    using std::ref;
    using std::reference_wrapper;

    // Copy the orbiter (controur only), we will have to work on it
    RawShape orbiter = sl::create<RawShape>(sl::getContour(cother));

    // Make the orbiter reverse oriented
    for(auto &v : sl::getContour(orbiter)) v = -v;

    // An egde with additional data for marking it
    struct MarkedEdge {
        Edge e; Radians turn_angle; bool is_turning_point;
        MarkedEdge() = default;
        MarkedEdge(const Edge& ed, Radians ta, bool tp):
            e(ed), turn_angle(ta), is_turning_point(tp) {}
    };

    // Container for marked edges
    using EdgeList = vector<MarkedEdge>;

    EdgeList A, B;

    // This is how an edge list is created from the polygons
    auto fillEdgeList = [](EdgeList& L, const RawShape& poly) {
        L.reserve(sl::contourVertexCount(poly));

        auto it = sl::cbegin(poly);
        auto nextit = std::next(it);

        L.emplace_back(Edge(*it, *nextit), 0, false);
        it++; nextit++;

        while(nextit != sl::cend(poly)) {
            Edge e(*it, *nextit);
            auto& L_prev = L.back();
            auto phi = L_prev.e.angleToXaxis();
            auto phi_prev = e.angleToXaxis();
            auto turn_angle = phi-phi_prev;
            if(turn_angle > Pi) turn_angle -= 2*Pi;
            L.emplace_back(e,
                           turn_angle,
                           signbit(turn_angle) != signbit(L_prev.turn_angle)
                           );
            it++; nextit++;
        }

        L.front().turn_angle = L.front().e.angleToXaxis() -
                               L.back().e.angleToXaxis();

        if(L.front().turn_angle > Pi) L.front().turn_angle -= 2*Pi;
    };

    // Fill the edgelists
    fillEdgeList(A, stationary);
    fillEdgeList(B, orbiter);

    // A reference to a marked edge that also knows its container
    struct MarkedEdgeRef {
        reference_wrapper<MarkedEdge> eref;
        reference_wrapper<vector<MarkedEdgeRef>> container;
        Coord dir = 1;
        inline Radians angleX() const { return eref.get().e.angleToXaxis(); }
        inline const Edge& edge() const { return eref.get().e; }
        inline Edge& edge() { return eref.get().e; }
        inline bool isTurningPoint() const {
            return eref.get().is_turning_point;
        }
        inline bool isFrom(const vector<MarkedEdgeRef>& cont ) {
            return &(container.get()) == &cont;
        }
        inline bool eq(const MarkedEdgeRef& mr) { return &eref == &(mr.eref); }

        MarkedEdgeRef(reference_wrapper<MarkedEdge> er,
                      reference_wrapper<vector<MarkedEdgeRef>> ec):
            eref(er), container(ec), dir(1) {}

        MarkedEdgeRef(reference_wrapper<MarkedEdge> er,
                      reference_wrapper<vector<MarkedEdgeRef>> ec,
                      Coord d):
            eref(er), container(ec), dir(d) {}
    };

    using EdgeRefList = vector<MarkedEdgeRef>;

    // Comparing two marked edges
    auto sortfn = [](const MarkedEdgeRef& e1, const MarkedEdgeRef& e2) {
        return e1.angleX() < e2.angleX();
    };

    EdgeRefList Aref, Bref;     // We create containers for the references
    Aref.reserve(A.size()); Bref.reserve(B.size());

    // Fill reference container for the stationary polygon
    std::for_each(A.begin(), A.end(), [&Aref](MarkedEdge& me) {
        Aref.emplace_back( ref(me), ref(Aref) );
    });

    // Fill reference container for the orbiting polygon
    std::for_each(B.begin(), B.end(), [&Bref](MarkedEdge& me) {
        Bref.emplace_back( ref(me), ref(Bref) );
    });

    struct EdgeGroup { typename EdgeRefList::const_iterator first, last; };

    auto mink = [sortfn]
            (const EdgeGroup& Q, const EdgeGroup& R, bool positive)
    {
        EdgeRefList sort_listQ(Q.first, Q.last);
        EdgeRefList sort_listR(R.first, R.last);

        // sort the containers of edge references
        sort(sort_listQ.begin(), sort_listQ.end(), sortfn);
        sort(sort_listR.begin(), sort_listR.end(), sortfn);

        EdgeRefList merged;
        EdgeRefList S, seq;
        merged.reserve(sort_listQ.size() + sort_listR.size());

//        auto edgeDir = [](Edge& e, Coord dir) {
//            Vertex d = Vertex{dir, dir};
//            e.first( e.first() * d );
//            e.second( e.second() * d );
//        };

        std::merge(sort_listQ.begin(), sort_listQ.end(),
                   sort_listR.begin(), sort_listR.end(),
                   std::back_inserter(merged), sortfn);

        const auto& Rcont = R.first->container.get();
        Coord dir = positive? 1 : -1;

        auto q = Q.first;
        S.push_back(*q);

        while(q != Q.last) {
            auto it = merged.begin();
            while(it != merged.end() && !(it->eq(*q) && q == Q.first) ) {
                if(it->isFrom(Rcont)) {
                    auto s = *it;
                    s.dir = dir;
                    S.push_back(s);
                }
                if(it->eq(*q)) {
                    S.push_back(*q);
                    if(it->isTurningPoint()) dir = -dir;
                }
                if(dir > 0) ++it;
                else --it;
            }
            ++q;
        }

        dir = 1;
        S.insert(S.begin(), *R.first);

        return seq;

        // TODO and step 5
    };

    EdgeGroup R{ Bref.begin(), Bref.begin() }, Q{ Aref.begin(), Aref.end() };
    auto it = Bref.begin();
    bool orientation = false;
    while(it != Bref.end())
        if(it->isTurningPoint()) {
            R = {R.last, it++}; mink(Q, R, orientation);
            orientation = !orientation;
        }

    // TODO step 6


    return Result(stationary, Vertex());
}

// Specializable NFP implementation class. Specialize it if you have a faster
// or better NFP implementation
template<class RawShape, NfpLevel nfptype>
struct NfpImpl {
    NfpResult<RawShape> operator()(const RawShape& sh, const RawShape& other)
    {
        static_assert(nfptype == NfpLevel::CONVEX_ONLY,
                      "Nfp::noFitPolygon() unimplemented!");

        // Libnest2D has a default implementation for convex polygons and will
        // use it if feasible.
        return nfpConvexOnly(sh, other);
    }
};

template<class RawShape> struct MaxNfpLevel {
    static const BP2D_CONSTEXPR NfpLevel value = NfpLevel::CONVEX_ONLY;
};

private:

// Do not specialize this...
template<class RawShape>
static inline bool _vsort(const TPoint<RawShape>& v1,
                          const TPoint<RawShape>& v2)
{
    using Coord = TCoord<TPoint<RawShape>>;
    Coord &&x1 = getX(v1), &&x2 = getX(v2), &&y1 = getY(v1), &&y2 = getY(v2);
    auto diff = y1 - y2;
    if(std::abs(diff) <= std::numeric_limits<Coord>::epsilon())
        return x1 < x2;

    return diff < 0;
}

};

}

#endif // GEOMETRIES_NOFITPOLYGON_HPP
