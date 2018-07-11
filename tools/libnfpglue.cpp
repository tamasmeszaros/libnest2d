//#ifndef NDEBUG
//#define NFP_DEBUG
//#endif

#include "libnfpglue.hpp"
#include "tools/libnfporb/libnfporb.hpp"

namespace libnest2d {

namespace  {
inline bool vsort(const libnfporb::point_t& v1, const libnfporb::point_t& v2)
{
    using Coord = libnfporb::coord_t;
    Coord x1 = v1.x_, x2 = v2.x_, y1 = v1.y_, y2 = v2.y_;
    auto diff = y1 - y2;
#ifdef LIBNFP_USE_RATIONAL
    long double diffv = diff.convert_to<long double>();
#else
    long double diffv = diff.val();
#endif
    if(std::abs(diffv) <=
            std::numeric_limits<Coord>::epsilon())
        return x1 < x2;

    return diff < 0;
}

TCoord<PointImpl> getX(const libnfporb::point_t& p) {
#ifdef LIBNFP_USE_RATIONAL
    return p.x_.convert_to<TCoord<PointImpl>>();
#else
    return static_cast<TCoord<PointImpl>>(std::round(p.x_.val()));
#endif
}

TCoord<PointImpl> getY(const libnfporb::point_t& p) {
#ifdef LIBNFP_USE_RATIONAL
    return p.y_.convert_to<TCoord<PointImpl>>();
#else
    return static_cast<TCoord<PointImpl>>(std::round(p.y_.val()));
#endif
}

}

PolygonImpl _nfp(const PolygonImpl &sh, const PolygonImpl &cother)
{
    using Coord = TCoord<PointImpl>;
    using Vertex = PointImpl;

    libnfporb::polygon_t pstat, porb;

    boost::geometry::convert(sh, pstat);
    boost::geometry::convert(cother, porb);

    auto nfp = libnfporb::generateNFP(pstat, porb, false);
    PolygonImpl ret;

    auto &ct = ShapeLike::getContour(ret);
    ct.reserve(nfp.front().size()+1);
    for(auto v : nfp.front()) ct.emplace_back(getX(v), getY(v));
    ct.push_back(ct.front());
    std::reverse(ct.begin(), ct.end());

    auto &rholes = ShapeLike::holes(ret);
    for(size_t hidx = 1; hidx < nfp.size(); ++hidx) {
        rholes.push_back({});
        auto& h = rholes.back();
        h.reserve(nfp[hidx].size()+1);

        for(auto& v : nfp[hidx]) h.emplace_back(getX(v), getY(v));
        h.push_back(h.front());
        std::reverse(h.begin(), h.end());
    }

    auto& cmp = vsort;
    std::sort(pstat.outer().begin(), pstat.outer().end(), cmp);
    std::sort(porb.outer().begin(), porb.outer().end(), cmp);

    // leftmost lower vertex of the stationary polygon
    auto& touch_sh = pstat.outer().back();
    // rightmost upper vertex of the orbiting polygon
    auto& touch_other = porb.outer().front();

    // Calculate the difference and move the orbiter to the touch position.
    auto dtouch = touch_sh - touch_other;
    auto _top_other = porb.outer().back() + dtouch;

    Vertex top_other(getX(_top_other), getY(_top_other));

    // Get the righmost upper vertex of the nfp and move it to the RMU of
    // the orbiter because they should coincide.
    auto&& top_nfp = Nfp::rightmostUpVertex(ret);
    auto dnfp = top_other - top_nfp;

    std::for_each(ShapeLike::begin(ret), ShapeLike::end(ret),
                  [&dnfp](Vertex& v) { v+= dnfp; } );

    for(auto& h : ShapeLike::holes(ret))
        std::for_each(h.begin(), h.end(), [&dnfp](Vertex& v) { v += dnfp; } );

    return ret;
}

PolygonImpl Nfp::NfpImpl<PolygonImpl, NfpLevel::CONVEX_ONLY>::operator()(
        const PolygonImpl &sh, const ClipperLib::PolygonImpl &cother)
{
//    return Nfp::nfpConvexOnly(sh, cother);
    return _nfp(sh, cother);
}

PolygonImpl Nfp::NfpImpl<PolygonImpl, NfpLevel::ONE_CONVEX>::operator()(
        const PolygonImpl &sh, const ClipperLib::PolygonImpl &cother)
{
    return _nfp(sh, cother);
}

PolygonImpl Nfp::NfpImpl<PolygonImpl, NfpLevel::BOTH_CONCAVE>::operator()(
        const PolygonImpl &sh, const ClipperLib::PolygonImpl &cother)
{
    return _nfp(sh, cother);
}

PolygonImpl
Nfp::NfpImpl<PolygonImpl, NfpLevel::BOTH_CONCAVE_WITH_HOLES>::operator()(
        const PolygonImpl &sh, const ClipperLib::PolygonImpl &cother)
{
    return _nfp(sh, cother);
}

}
