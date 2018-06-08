#ifndef NOFITPOLY_HPP
#define NOFITPOLY_HPP

#include <map>
#include "placer_boilerplate.hpp"
#include "../geometries_nfp.hpp"
#include <libnest2d/optimizers/genetic.hpp>
#include <libnest2d/optimizers/simplex.hpp>

namespace libnest2d { namespace strategies {

template<class RawShape>
struct NfpPConfig {

    enum class Alignment {
        CENTER,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        TOP_LEFT,
        TOP_RIGHT,
    };

    std::vector<Radians> rotations;
    Alignment alignment;
    NfpPConfig(): rotations(1, 0.0), alignment(Alignment::BOTTOM_LEFT) {}
    bool use_solver = true;
};

// A class for getting a point on the circumference of the polygon (in log time)
template<class RawShape> class EdgeCache {
    using Vertex = TPoint<RawShape>;
    using Coord = TCoord<Vertex>;
    using Edge = _Segment<Vertex>;

    std::vector<Edge> emap_;
    std::vector<double> distances_;
    double full_distance_ = 0;

    void createCache(const RawShape& sh) {
        auto first = ShapeLike::cbegin(sh);
        auto next = first + 1;
        auto endit = ShapeLike::cend(sh);

        distances_.reserve(ShapeLike::contourVertexCount(sh));

        while(next != endit) {
            emap_.emplace_back(*(first++), *(next++));
            full_distance_ += emap_.back().length();
            distances_.push_back(full_distance_);
        }
    }

public:

    inline EdgeCache() = default;

    inline EdgeCache(const _Item<RawShape>& item) {
        createCache(item.transformedShape());
    }

    inline EdgeCache(const RawShape& sh) { createCache(sh); }

    /**
     * @brief Get a point on the circumference of a polygon.
     * @param distance A relative distance from the starting point to the end.
     * Can be from 0.0 to 1.0 where 0.0 is the starting point and 1.0 is the
     * closing point (which should be eqvivalent with the starting point with
     * closed polygons).
     * @return Returns the coordinates of the point lying on the polygon
     * circumference.
     */
    inline Vertex coords(double distance) {
        assert(distance >= .0 && distance <= 1.0);

        // distance is from 0.0 to 1.0, we scale it up to the full length of
        // the circumference
        double d = distance*full_distance_;

        // Magic: we find the right edge in log time
        auto it = std::lower_bound(distances_.begin(), distances_.end(), d);
        auto idx = it - distances_.begin();     // get the index of the edge
        auto edge = emap_[idx];   // extrac the edge

        // Get the remaining distance on the target edge
        auto ed = d - (idx > 0 ? *std::prev(it) : 0 );
        auto angle = edge.angleToXaxis();
        Vertex ret = edge.first();

        // Get the point on the edge which lies in ed distance from the start
        ret += { static_cast<Coord>(std::round(ed*std::cos(angle))),
                 static_cast<Coord>(std::round(ed*std::sin(angle))) };

        return ret;
    }

    inline double circumference() const BP2D_NOEXCEPT { return full_distance_; }
};

// Nfp for a bunch of polygons. If the polygons are convex, the nfp calculated
// for trsh can be the union of nfp-s calculated with each polygon
template<class RawShape, class Container>
Nfp::Shapes<RawShape> nfp(const Container& polygons, const RawShape& trsh )
{
    using Item = _Item<RawShape>;

    Nfp::Shapes<RawShape> nfps;

    for(Item& sh : polygons) {
        auto subnfp = Nfp::noFitPolygon(sh.transformedShape(),
                                        trsh);
        #ifndef NDEBUG
            auto vv = ShapeLike::isValid(sh.transformedShape());
            assert(vv.first);

            auto vnfp = ShapeLike::isValid(subnfp);
            assert(vnfp.first);
        #endif

        nfps = Nfp::merge(nfps, subnfp);
    }

    return nfps;
}

template<class RawShape>
class _NofitPolyPlacer: public PlacerBoilerplate<_NofitPolyPlacer<RawShape>,
        RawShape, _Box<TPoint<RawShape>>, NfpPConfig<RawShape>> {

    using Base = PlacerBoilerplate<_NofitPolyPlacer<RawShape>,
    RawShape, _Box<TPoint<RawShape>>, NfpPConfig<RawShape>>;

    DECLARE_PLACER(Base)

    using Box = _Box<TPoint<RawShape>>;

public:

    inline explicit _NofitPolyPlacer(const BinType& bin): Base(bin) {}

    PackResult trypack(Item& item) {

        PackResult ret;

        bool can_pack = false;

        if(items_.empty()) {
            setInitialPosition(item);
            can_pack = item.isInside(bin_);
        } else {
            if(config_.use_solver) {
                // place the new item outside of the print bed to make sure it is
                // disjuct from the current merged pile
                placeOutsideOfBin(item);

                auto trsh = item.transformedShape();

                auto nfps = nfp(items_, trsh);
                auto iv = Nfp::referenceVertex(trsh);
                auto startpos = item.translation();
                std::vector<EdgeCache<RawShape>> ecache;
                ecache.reserve(nfps.size());
                for(auto& nfp : nfps ) ecache.emplace_back(nfp);

                auto getNfpPoint = [&nfps, &ecache](double relpos) {
                    auto nfp_idx = static_cast<unsigned>(relpos / nfps.size());
                    if(nfp_idx >= nfps.size()) nfp_idx = nfps.size()-1;
                    auto p = relpos - std::floor(relpos / nfps.size());
                    return ecache[nfp_idx].coords(p);
                };

                auto objfunc = [&] (std::tuple<double> relpos)
                {
                    Vertex v = getNfpPoint(std::get<0>(relpos));
                    auto d = v - iv;
                    d += startpos;
                    item.translation(d);

                    if( item.isInside(bin_) ) {
                        Nfp::Shapes<RawShape> m;
                        m.reserve(items_.size());

                        for(Item& pi : items_)
                            m.emplace_back(pi.transformedShape());

                        m.emplace_back(item.transformedShape());
                        double occupied_area = 0;
                        std::for_each(m.begin(), m.end(),
                                      [&occupied_area](const RawShape& mshape){
                           occupied_area +=  ShapeLike::area(mshape);
                        });

                        auto ch = ShapeLike::convexHull(m);
                        auto circumf = EdgeCache<RawShape>(ch).circumference();
                        double pack_rate = occupied_area/ShapeLike::area(ch);

                        auto score = std::sqrt(circumf * (1.0 - pack_rate));
                        return score;
                    }

                    return std::nan(""); // score = pack efficiency
                };

                opt::StopCriteria stopcr;
                stopcr.max_iterations = 1000;
                opt::TOptimizer<opt::Method::GENETIC> solver(stopcr);

                try {

                    auto result = solver.optimize_min(objfunc,
                                    opt::initvals(0.0),
                                    opt::bound(0.0, 1.0*nfps.size())
                                    );

                    if(!std::isnan(result.score) ) {

                        auto d = getNfpPoint(std::get<0>(result.optimum)) - iv;
                        d += startpos;
                        item.translation(d);
                        if(item.isInside(bin_)) can_pack = true;
                    }
                } catch(std::exception& ) {}

            } else {

                auto initial_tr = item.translation();
                auto initial_rot = item.rotation();
                double min_area = std::numeric_limits<double>::max();
                Vertex final_tr = {0, 0};
                Radians final_rot = 0;
                Nfp::Shapes<RawShape> nfps;

                for(auto rot : config_.rotations) {

                    item.translation(initial_tr);
                    item.rotation(initial_rot + rot);

                    // place the new item outside of the print bed to make sure it is
                    // disjuct from the current merged pile
                    placeOutsideOfBin(item);

                    auto trsh = item.transformedShape();

                    #ifndef NDEBUG
                        auto v = ShapeLike::isValid(trsh);
                        assert(v.first);
                    #endif

                    nfps = nfp(items_, trsh);

                    auto iv = Nfp::referenceVertex(trsh);

                    auto startpos = item.translation();

                    // place item on each the edge of this nfp
                    for(auto& nfp : nfps)
                    ShapeLike::foreachContourVertex(nfp, [&]
                                                    (Vertex& v)
                    {
                        auto d = v - iv;
                        d += startpos;
                        item.translation(d);

                        if( item.isInside(bin_) ) {
                            Nfp::Shapes<RawShape> m;
                            m.reserve(items_.size());

                            for(Item& pi : items_)
                                m.emplace_back(pi.transformedShape());

                            m.emplace_back(item.transformedShape());

                            double occupied_area = 0;
                            std::for_each(m.begin(), m.end(),
                                          [&occupied_area](const RawShape& mshape){
                               occupied_area +=  ShapeLike::area(mshape);
                            });

                            auto ch = ShapeLike::convexHull(m);
                            auto circumf = EdgeCache<RawShape>(ch).circumference();
                            double pack_rate = occupied_area/ShapeLike::area(ch);

                            auto a = std::sqrt(circumf * (1.0 - pack_rate));

                            if(a < min_area) {
                                can_pack = true;
                                min_area = a;
                                final_tr = d;
                                final_rot = initial_rot + rot;
                            }
                        }
                    });
                }

                #ifndef NDEBUG
                            if(can_pack) for(auto&nfp : nfps) {
                                auto val = ShapeLike::isValid(nfp);
                                assert(val.first);
                #ifdef DEBUG_EXPORT_NFP
                                Base::debug_items_.emplace_back(nfp);
                #endif
                            }
                #endif

                item.translation(final_tr);
                item.rotation(final_rot);
            }
        }

        if(can_pack) {
            ret = PackResult(item);
        }

        return ret;
    }

private:

    void setInitialPosition(Item& item) {
        Box&& bb = item.boundingBox();
        Vertex ci, cb;

        switch(config_.alignment) {
        case Config::Alignment::CENTER: {
            ci = bb.center();
            cb = bin_.center();
            break;
        }
        case Config::Alignment::BOTTOM_LEFT: {
            ci = bb.minCorner();
            cb = bin_.minCorner();
            break;
        }
        case Config::Alignment::BOTTOM_RIGHT: {
            ci = {getX(bb.maxCorner()), getY(bb.minCorner())};
            cb = {getX(bin_.maxCorner()), getY(bin_.minCorner())};
            break;
        }
        case Config::Alignment::TOP_LEFT: {
            ci = {getX(bb.minCorner()), getY(bb.maxCorner())};
            cb = {getX(bin_.minCorner()), getY(bin_.maxCorner())};
            break;
        }
        case Config::Alignment::TOP_RIGHT: {
            ci = bb.maxCorner();
            cb = bin_.maxCorner();
            break;
        }
        }

        auto d = cb - ci;
        item.translate(d);
    }

    void placeOutsideOfBin(Item& item) {
        auto bb = item.boundingBox();
        Box binbb = ShapeLike::boundingBox<RawShape>(bin_);

        Vertex v = { getX(bb.maxCorner()), getY(bb.minCorner()) };

        Coord dx = getX(binbb.maxCorner()) - getX(v);
        Coord dy = getY(binbb.maxCorner()) - getY(v);

        item.translate({dx, dy});
    }

};


}
}

#endif // NOFITPOLY_H
