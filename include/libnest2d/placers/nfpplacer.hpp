#ifndef NOFITPOLY_HPP
#define NOFITPOLY_HPP

#include <cassert>

// For parallel for
#include <functional>
#include <iterator>
#include <future>
#include <atomic>

#ifndef NDEBUG
#include <iostream>
#endif
#include <libnest2d/geometry_traits_nfp.hpp>
#include <libnest2d/optimizer.hpp>

#include "placer_boilerplate.hpp"

// temporary
//#include "../tools/svgtools.hpp"

#include <libnest2d/parallel.hpp>

namespace libnest2d {
namespace placers {

template<class RawShape>
struct NfpPConfig {

    using ItemGroup = _ItemGroup<RawShape>;

    enum class Alignment {
        CENTER,
        BOTTOM_LEFT,
        BOTTOM_RIGHT,
        TOP_LEFT,
        TOP_RIGHT,
        DONT_ALIGN      //!> Warning: parts may end up outside the bin with the
                        //! default object function.
    };

    /// Which angles to try out for better results.
    std::vector<Radians> rotations;

    /// Where to align the resulting packed pile.
    Alignment alignment;

    /// Where to start putting objects in the bin.
    Alignment starting_point;

    /**
     * @brief A function object representing the fitting function in the
     * placement optimization process. (Optional)
     *
     * This is the most versatile tool to configure the placer. The fitting
     * function is evaluated many times when a new item is being placed into the
     * bin. The output should be a rated score of the new item's position.
     *
     * This is not a mandatory option as there is a default fitting function
     * that will optimize for the best pack efficiency. With a custom fitting
     * function you can e.g. influence the shape of the arranged pile.
     *
     * \param item The only parameter is the candidate item which has info
     * about its current position. Your job is to rate this position compared to
     * the already packed items.
     *
     */
    std::function<double(const _Item<RawShape>&)> object_function;

    /**
     * @brief The quality of search for an optimal placement.
     * This is a compromise slider between quality and speed. Zero is the
     * fast and poor solution while 1.0 is the slowest but most accurate.
     */
    float accuracy = 0.65f;

    /**
     * @brief If you want to see items inside other item's holes, you have to
     * turn this switch on.
     *
     * This will only work if a suitable nfp implementation is provided.
     * The library has no such implementation right now.
     */
    bool explore_holes = false;

    /**
     * @brief If true, use all CPUs available. Run on a single core otherwise.
     */
    bool parallel = true;

    /**
     * @brief before_packing Callback that is called just before a search for
     * a new item's position is started. You can use this to create various
     * cache structures and update them between subsequent packings.
     *
     * \param merged pile A polygon that is the union of all items in the bin.
     *
     * \param pile The items parameter is a container with all the placed
     * polygons excluding the current candidate. You can for instance check the
     * alignment with the candidate item or do anything else.
     *
     * \param remaining A container with the remaining items waiting to be
     * placed. You can use some features about the remaining items to alter the
     * score of the current placement. If you know that you have to leave place
     * for other items as well, that might influence your decision about where
     * the current candidate should be placed. E.g. imagine three big circles
     * which you want to place into a box: you might place them in a triangle
     * shape which has the maximum pack density. But if there is a 4th big
     * circle than you won't be able to pack it. If you knew apriori that
     * there four circles are to be placed, you would have placed the first 3
     * into an L shape. This parameter can be used to make these kind of
     * decisions (for you or a more intelligent AI).
     */
    std::function<void(const nfp::Shapes<RawShape>&, // merged pile
                       const ItemGroup&,             // packed items
                       const ItemGroup&              // remaining items
                       )> before_packing;

    NfpPConfig(): rotations({0.0, Pi/2.0, Pi, 3*Pi/2}),
        alignment(Alignment::CENTER), starting_point(Alignment::CENTER) {}
};

/**
 * A class for getting a point on the circumference of the polygon (in log time)
 *
 * This is a transformation of the provided polygon to be able to pinpoint
 * locations on the circumference. The optimizer will pass a floating point
 * value e.g. within <0,1> and we have to transform this value quickly into a
 * coordinate on the circumference. By definition 0 should yield the first
 * vertex and 1.0 would be the last (which should coincide with first).
 *
 * We also have to make this work for the holes of the captured polygon.
 */
template<class RawShape> class EdgeCache {
    using Vertex = TPoint<RawShape>;
    using Coord = TCoord<Vertex>;
    using Edge = _Segment<Vertex>;

    struct ContourCache {
        mutable std::vector<double> corners;
        std::vector<Edge> emap;
        std::vector<double> distances;
        double full_distance = 0;
    } contour_;

    std::vector<ContourCache> holes_;

    double accuracy_ = 1.0;
    
    static double length(const Edge &e) 
    { 
        return std::sqrt(e.template sqlength<double>());
    }

    void createCache(const RawShape& sh) {
        {   // For the contour
            auto first = shapelike::cbegin(sh);
            auto next = std::next(first);
            auto endit = shapelike::cend(sh);

            contour_.distances.reserve(shapelike::contourVertexCount(sh));

            while(next != endit) {
                contour_.emap.emplace_back(*(first++), *(next++));
                contour_.full_distance += length(contour_.emap.back());
                contour_.distances.emplace_back(contour_.full_distance);
            }
        }

        for(auto& h : shapelike::holes(sh)) { // For the holes
            auto first = h.begin();
            auto next = std::next(first);
            auto endit = h.end();

            ContourCache hc;
            hc.distances.reserve(endit - first);

            while(next != endit) {
                hc.emap.emplace_back(*(first++), *(next++));
                hc.full_distance += length(hc.emap.back());
                hc.distances.emplace_back(hc.full_distance);
            }

            holes_.emplace_back(std::move(hc));
        }
    }

    size_t stride(const size_t N) const {
        using std::round;
        using std::pow;

        return static_cast<size_t>(
                    round(N/pow(N, pow(accuracy_, 1.0/3.0)))
                );
    }

    void fetchCorners() const {
        if(!contour_.corners.empty()) return;

        const auto N = contour_.distances.size();
        const auto S = stride(N);

        contour_.corners.reserve(N / S + 1);
        contour_.corners.emplace_back(0.0);
        auto N_1 = N-1;
        contour_.corners.emplace_back(0.0);
        for(size_t i = 0; i < N_1; i += S) {
            contour_.corners.emplace_back(
                    contour_.distances.at(i) / contour_.full_distance);
        }
    }

    void fetchHoleCorners(unsigned hidx) const {
        auto& hc = holes_[hidx];
        if(!hc.corners.empty()) return;

        const auto N = hc.distances.size();
        auto N_1 = N-1;
        const auto S = stride(N);
        hc.corners.reserve(N / S + 1);
        hc.corners.emplace_back(0.0);
        for(size_t i = 0; i < N_1; i += S) {
            hc.corners.emplace_back(
                    hc.distances.at(i) / hc.full_distance);
        }
    }

    inline Vertex coords(const ContourCache& cache, double distance) const {
        assert(distance >= .0 && distance <= 1.0);

        // distance is from 0.0 to 1.0, we scale it up to the full length of
        // the circumference
        double d = distance*cache.full_distance;

        auto& distances = cache.distances;

        // Magic: we find the right edge in log time
        auto it = std::lower_bound(distances.begin(), distances.end(), d);
        auto idx = it - distances.begin();      // get the index of the edge
        auto edge = cache.emap[idx];         // extrac the edge

        // Get the remaining distance on the target edge
        auto ed = d - (idx > 0 ? *std::prev(it) : 0 );
        auto angle = edge.angleToXaxis();
        Vertex ret = edge.first();

        // Get the point on the edge which lies in ed distance from the start
        ret += { static_cast<Coord>(std::round(ed*std::cos(angle))),
                 static_cast<Coord>(std::round(ed*std::sin(angle))) };

        return ret;
    }

public:

    using iterator = std::vector<double>::iterator;
    using const_iterator = std::vector<double>::const_iterator;

    inline EdgeCache() = default;

    inline EdgeCache(const _Item<RawShape>& item)
    {
        createCache(item.transformedShape());
    }

    inline EdgeCache(const RawShape& sh)
    {
        createCache(sh);
    }

    /// Resolution of returned corners. The stride is derived from this value.
    void accuracy(double a /* within <0.0, 1.0>*/) { accuracy_ = a; }

    /**
     * @brief Get a point on the circumference of a polygon.
     * @param distance A relative distance from the starting point to the end.
     * Can be from 0.0 to 1.0 where 0.0 is the starting point and 1.0 is the
     * closing point (which should be eqvivalent with the starting point with
     * closed polygons).
     * @return Returns the coordinates of the point lying on the polygon
     * circumference.
     */
    inline Vertex coords(double distance) const {
        return coords(contour_, distance);
    }

    inline Vertex coords(unsigned hidx, double distance) const {
        assert(hidx < holes_.size());
        return coords(holes_[hidx], distance);
    }

    inline double circumference() const BP2D_NOEXCEPT {
        return contour_.full_distance;
    }

    inline double circumference(unsigned hidx) const BP2D_NOEXCEPT {
        return holes_[hidx].full_distance;
    }

    /// Get the normalized distance values for each vertex
    inline const std::vector<double>& corners() const BP2D_NOEXCEPT {
        fetchCorners();
        return contour_.corners;
    }

    /// corners for a specific hole
    inline const std::vector<double>&
    corners(unsigned holeidx) const BP2D_NOEXCEPT {
        fetchHoleCorners(holeidx);
        return holes_[holeidx].corners;
    }

    /// The number of holes in the abstracted polygon
    inline size_t holeCount() const BP2D_NOEXCEPT { return holes_.size(); }

};

template<nfp::NfpLevel lvl>
struct Lvl { static const nfp::NfpLevel value = lvl; };

template<class RawShape>
inline void correctNfpPosition(nfp::NfpResult<RawShape>& nfp,
                               const _Item<RawShape>& stationary,
                               const _Item<RawShape>& orbiter)
{
    // The provided nfp is somewhere in the dark. We need to get it
    // to the right position around the stationary shape.
    // This is done by choosing the leftmost lowest vertex of the
    // orbiting polygon to be touched with the rightmost upper
    // vertex of the stationary polygon. In this configuration, the
    // reference vertex of the orbiting polygon (which can be dragged around
    // the nfp) will be its rightmost upper vertex that coincides with the
    // rightmost upper vertex of the nfp. No proof provided other than Jonas
    // Lindmark's reasoning about the reference vertex of nfp in his thesis
    // ("No fit polygon problem" - section 2.1.9)

    auto touch_sh = stationary.rightmostTopVertex();
    auto touch_other = orbiter.leftmostBottomVertex();
    auto dtouch = touch_sh - touch_other;
    auto top_other = orbiter.rightmostTopVertex() + dtouch;
    auto dnfp = top_other - nfp.second; // nfp.second is the nfp reference point
    shapelike::translate(nfp.first, dnfp);
}

template<class RawShape>
inline void correctNfpPosition(nfp::NfpResult<RawShape>& nfp,
                               const RawShape& stationary,
                               const _Item<RawShape>& orbiter)
{
    auto touch_sh = nfp::rightmostUpVertex(stationary);
    auto touch_other = orbiter.leftmostBottomVertex();
    auto dtouch = touch_sh - touch_other;
    auto top_other = orbiter.rightmostTopVertex() + dtouch;
    auto dnfp = top_other - nfp.second;
    shapelike::translate(nfp.first, dnfp);
}

template<class RawShape, class Circle = _Circle<TPoint<RawShape>> >
Circle minimizeCircle(const RawShape& sh) {
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;

    auto& ctr = sl::contour(sh);
    if(ctr.empty()) return {{0, 0}, 0};

    auto bb = sl::boundingBox(sh);
    auto capprx = bb.center();
    auto rapprx = pl::distance(bb.minCorner(), bb.maxCorner());


    opt::StopCriteria stopcr;
    stopcr.max_iterations = 30;
    stopcr.relative_score_difference = 1e-3;
    opt::TOptimizer<opt::Method::L_SUBPLEX> solver(stopcr);

    std::vector<double> dists(ctr.size(), 0);

    auto result = solver.optimize_min(
        [capprx, rapprx, &ctr, &dists](double xf, double yf) {
            auto xt = Coord( std::round(getX(capprx) + rapprx*xf) );
            auto yt = Coord( std::round(getY(capprx) + rapprx*yf) );

            Point centr(xt, yt);

            unsigned i = 0;
            for(auto v : ctr) {
                dists[i++] = pl::distance(v, centr);
            }

            auto mit = std::max_element(dists.begin(), dists.end());

            assert(mit != dists.end());

            return *mit;
        },
        opt::initvals(0.0, 0.0),
        opt::bound(-1.0, 1.0), opt::bound(-1.0, 1.0)
    );

    double oxf = std::get<0>(result.optimum);
    double oyf = std::get<1>(result.optimum);
    auto xt = Coord( std::round(getX(capprx) + rapprx*oxf) );
    auto yt = Coord( std::round(getY(capprx) + rapprx*oyf) );

    Point cc(xt, yt);
    auto r = result.score;

    return {cc, r};
}

template<class RawShape>
_Circle<TPoint<RawShape>> boundingCircle(const RawShape& sh) {
    return minimizeCircle(sh);
}

template<class RawShape, class TBin = _Box<TPoint<RawShape>>>
class _NofitPolyPlacer: public PlacerBoilerplate<_NofitPolyPlacer<RawShape, TBin>,
        RawShape, TBin, NfpPConfig<RawShape>> {

    using Base = PlacerBoilerplate<_NofitPolyPlacer<RawShape, TBin>,
    RawShape, TBin, NfpPConfig<RawShape>>;

    DECLARE_PLACER(Base)

    using Box = _Box<TPoint<RawShape>>;

    using MaxNfpLevel = nfp::MaxNfpLevel<RawShape>;

    // Norming factor for the optimization function
    const double norm_;

public:

    using Pile = nfp::Shapes<RawShape>;

    inline explicit _NofitPolyPlacer(const BinType& bin):
        Base(bin),
        norm_(std::sqrt(sl::area(bin)))
    {
        // In order to not have items out of bin, it will be shrinked by an
        // very little empiric offset value.
        // sl::offset(bin_, 1e-5 * norm_);
    }

    _NofitPolyPlacer(const _NofitPolyPlacer&) = default;
    _NofitPolyPlacer& operator=(const _NofitPolyPlacer&) = default;

#ifndef BP2D_COMPILER_MSVC12 // MSVC2013 does not support default move ctors
    _NofitPolyPlacer(_NofitPolyPlacer&&) = default;
    _NofitPolyPlacer& operator=(_NofitPolyPlacer&&) = default;
#endif

    static inline double overfit(const Box& bb, const RawShape& bin) {
        auto bbin = sl::boundingBox(bin);
        auto d = bbin.center() - bb.center();
        _Rectangle<RawShape> rect(bb.width(), bb.height());
        rect.translate(bb.minCorner() + d);
        return sl::isInside(rect.transformedShape(), bin) ? -1.0 : 1;
    }

    static inline double overfit(const RawShape& chull, const RawShape& bin) {
        auto bbch = sl::boundingBox(chull);
        auto bbin = sl::boundingBox(bin);
        auto d =  bbch.center() - bbin.center();
        auto chullcpy = chull;
        sl::translate(chullcpy, d);
        return sl::isInside(chullcpy, bin) ? -1.0 : 1.0;
    }

    static inline double overfit(const RawShape& chull, const Box& bin)
    {
        auto bbch = sl::boundingBox(chull);
        return overfit(bbch, bin);
    }

    static inline double overfit(const Box& bb, const Box& bin)
    {
        auto Bw = bin.width();
        auto Bh = bin.height();
        auto mBw = -Bw;
        auto mBh = -Bh;
        auto wdiff = double(bb.width()) + mBw;
        auto hdiff = double(bb.height()) + mBh;
        double diff = 0;
        if(wdiff > 0) diff += wdiff;
        if(hdiff > 0) diff += hdiff;
        return diff;
    }

    static inline double overfit(const Box& bb, const _Circle<Vertex>& bin)
    {
        double boxr = 0.5*pl::distance(bb.minCorner(), bb.maxCorner());
        double diff = boxr - bin.radius();
        return diff;
    }

    static inline double overfit(const RawShape& chull,
                                const _Circle<Vertex>& bin)
    {
        double r = boundingCircle(chull).radius();
        double diff = r - bin.radius();
        return diff;
    }

    template<class Range = ConstItemRange<typename Base::DefaultIter>>
    PackResult trypack(Item& item,
                        const Range& remaining = Range()) {
        auto result = _trypack(item, remaining);

        // Experimental
        // if(!result) repack(item, result);

        return result;
    }

    ~_NofitPolyPlacer() {
        clearItems();
    }

    inline void clearItems() {
        finalAlign(bin_);
        Base::clearItems();
    }

private:

    using Shapes = TMultiShape<RawShape>;

    Shapes calcnfp(const Item &trsh, Lvl<nfp::NfpLevel::CONVEX_ONLY>)
    {
        using namespace nfp;

        Shapes nfps(items_.size());

        // /////////////////////////////////////////////////////////////////////
        // TODO: this is a workaround and should be solved in Item with mutexes
        // guarding the mutable members when writing them.
        // /////////////////////////////////////////////////////////////////////
        trsh.transformedShape();
        trsh.referenceVertex();
        trsh.rightmostTopVertex();
        trsh.leftmostBottomVertex();

        for(Item& itm : items_) {
            itm.transformedShape();
            itm.referenceVertex();
            itm.rightmostTopVertex();
            itm.leftmostBottomVertex();
        }
        // /////////////////////////////////////////////////////////////////////

        __parallel::enumerate(items_.begin(), items_.end(),
                              [&nfps, &trsh](const Item& sh, size_t n)
        {
            auto& fixedp = sh.transformedShape();
            auto& orbp = trsh.transformedShape();
            auto subnfp_r = noFitPolygon<NfpLevel::CONVEX_ONLY>(fixedp, orbp);
            correctNfpPosition(subnfp_r, sh, trsh);
            nfps[n] = subnfp_r.first;
        });

        return nfp::merge(nfps);
    }


    template<class Level>
    Shapes calcnfp(const Item &trsh, Level)
    { // Function for arbitrary level of nfp implementation
        using namespace nfp;

        Shapes nfps;

        auto& orb = trsh.transformedShape();
        bool orbconvex = trsh.isContourConvex();

        for(Item& sh : items_) {
            nfp::NfpResult<RawShape> subnfp;
            auto& stat = sh.transformedShape();

            if(sh.isContourConvex() && orbconvex)
                subnfp = nfp::noFitPolygon<NfpLevel::CONVEX_ONLY>(stat, orb);
            else if(orbconvex)
                subnfp = nfp::noFitPolygon<NfpLevel::ONE_CONVEX>(stat, orb);
            else
                subnfp = nfp::noFitPolygon<Level::value>(stat, orb);

            correctNfpPosition(subnfp, sh, trsh);

            nfps = nfp::merge(nfps, subnfp.first);
        }

        return nfps;
    }

    // Very much experimental
    void repack(Item& item, PackResult& result) {

        if((sl::area(bin_) - this->filledArea()) >= item.area()) {
            auto prev_func = config_.object_function;

            unsigned iter = 0;
            ItemGroup backup_rf = items_;
            std::vector<Item> backup_cpy;
            for(Item& itm : items_) backup_cpy.emplace_back(itm);

            auto ofn = [this, &item, &result, &iter, &backup_cpy, &backup_rf]
                    (double ratio)
            {
                auto& bin = bin_;
                iter++;
                config_.object_function = [bin, ratio](
                        nfp::Shapes<RawShape>& pile,
                        const Item& item,
                        const ItemGroup& /*remaining*/)
                {
                    pile.emplace_back(item.transformedShape());
                    auto ch = sl::convexHull(pile);
                    auto pbb = sl::boundingBox(pile);
                    pile.pop_back();

                    double parea = 0.5*(sl::area(ch) + sl::area(pbb));

                    double pile_area = std::accumulate(
                                pile.begin(), pile.end(), item.area(),
                                [](double sum, const RawShape& sh){
                        return sum + sl::area(sh);
                    });

                    // The pack ratio -- how much is the convex hull occupied
                    double pack_rate = (pile_area)/parea;

                    // ratio of waste
                    double waste = 1.0 - pack_rate;

                    // Score is the square root of waste. This will extend the
                    // range of good (lower) values and shrink the range of bad
                    // (larger) values.
                    auto wscore = std::sqrt(waste);


                    auto ibb = item.boundingBox();
                    auto bbb = sl::boundingBox(bin);
                    auto c = ibb.center();
                    double norm = 0.5*pl::distance(bbb.minCorner(),
                                                   bbb.maxCorner());

                    double dscore = pl::distance(c, pbb.center()) / norm;

                    return ratio*wscore + (1.0 - ratio) * dscore;
                };

                auto bb = sl::boundingBox(bin);
                double norm = bb.width() + bb.height();

                auto items = items_;
                clearItems();
                auto it = items.begin();
                while(auto pr = _trypack(*it++)) {
                    this->accept(pr); if(it == items.end()) break;
                }

                auto count_diff = items.size() - items_.size();
                double score = count_diff;

                if(count_diff == 0) {
                    result = _trypack(item);

                    if(result) {
                        std::cout << "Success" << std::endl;
                        score = 0.0;
                    } else {
                        score += result.overfit() / norm;
                    }
                } else {
                    result = PackResult();
                    items_ = backup_rf;
                    for(unsigned i = 0; i < items_.size(); i++) {
                        items_[i].get() = backup_cpy[i];
                    }
                }

                std::cout << iter << " repack result: " << score << " "
                          << ratio << " " << count_diff << std::endl;

                return score;
            };

                opt::StopCriteria stopcr;
                stopcr.max_iterations = 30;
                stopcr.stop_score = 1e-20;
                opt::TOptimizer<opt::Method::L_SUBPLEX> solver(stopcr);
                solver.optimize_min(ofn, opt::initvals(0.5),
                                    opt::bound(0.0, 1.0));

            // optimize
            config_.object_function = prev_func;
        }
    }

    struct Optimum {
        double relpos;
        unsigned nfpidx;
        int hidx;
        Optimum(double pos, unsigned nidx):
            relpos(pos), nfpidx(nidx), hidx(-1) {}
        Optimum(double pos, unsigned nidx, int holeidx):
            relpos(pos), nfpidx(nidx), hidx(holeidx) {}
    };

    class Optimizer: public opt::TOptimizer<opt::Method::L_SUBPLEX> {
    public:
        Optimizer(float accuracy = 1.f) {
            opt::StopCriteria stopcr;
            stopcr.max_iterations = unsigned(std::floor(1000 * accuracy));
            stopcr.relative_score_difference = 1e-20;
            this->stopcr_ = stopcr;
        }
    };

    using Edges = EdgeCache<RawShape>;

    template<class Range = ConstItemRange<typename Base::DefaultIter>>
    PackResult _trypack(
            Item& item,
            const Range& remaining = Range()) {

        PackResult ret;

        bool can_pack = false;
        double best_overfit = std::numeric_limits<double>::max();

        ItemGroup remlist;
        if(remaining.valid) {
            remlist.insert(remlist.end(), remaining.from, remaining.to);
        }

        if(std::all_of(items_.begin(), items_.end(),
                [](const Item& item) { return item.isDisallowedArea(); })) {
            setInitialPosition(item);
            best_overfit = overfit(item.transformedShape(), bin_);
            can_pack = best_overfit <= 0;
        } else {

            double global_score = std::numeric_limits<double>::max();

            auto initial_tr = item.translation();
            auto initial_rot = item.rotation();
            Vertex final_tr = {0, 0};
            Radians final_rot = initial_rot;
            Shapes nfps;

            for(auto rot : config_.rotations) {

                item.translation(initial_tr);
                item.rotation(initial_rot + rot);
                item.boundingBox(); // fill the bb cache

                // place the new item outside of the print bed to make sure
                // it is disjunct from the current merged pile
                placeOutsideOfBin(item);

                nfps = calcnfp(item, Lvl<MaxNfpLevel::value>());

                auto iv = item.referenceVertex();

                auto startpos = item.translation();

                std::vector<Edges> ecache;
                ecache.reserve(nfps.size());

                for(auto& nfp : nfps ) {
                    ecache.emplace_back(nfp);
                    ecache.back().accuracy(config_.accuracy);
                }

                Shapes pile;
                pile.reserve(items_.size()+1);
                // double pile_area = 0;
                for(Item& mitem : items_) {
                    pile.emplace_back(mitem.transformedShape());
                    // pile_area += mitem.area();
                }

                auto merged_pile = nfp::merge(pile);
                auto& bin = bin_;
                double norm = norm_;
                auto pbb = sl::boundingBox(merged_pile);
                auto binbb = sl::boundingBox(bin);

                // This is the kernel part of the object function that is
                // customizable by the library client
                std::function<double(const Item&)> _objfunc;
                if(config_.object_function) _objfunc = config_.object_function;
                else {

                    // Inside check has to be strict if no alignment was enabled
                    std::function<double(const Box&)> ins_check;
                    if(config_.alignment == Config::Alignment::DONT_ALIGN)
                        ins_check = [&binbb, norm](const Box& fullbb) {
                            double ret = 0;
                            if(!sl::isInside(fullbb, binbb))
                                ret += norm;
                            return ret;
                        };
                    else
                        ins_check = [&bin](const Box& fullbb) {
                            double miss = overfit(fullbb, bin);
                            miss = miss > 0? miss : 0;
                            return std::pow(miss, 2);
                        };

                    _objfunc = [norm, binbb, pbb, ins_check](const Item& item)
                    {
                        auto ibb = item.boundingBox();
                        auto fullbb = sl::boundingBox(pbb, ibb);

                        double score = pl::distance(ibb.center(),
                                                    binbb.center());
                        score /= norm;

                        score += ins_check(fullbb);

                        return score;
                    };
                }

                // Our object function for placement
                auto rawobjfunc = [_objfunc, iv, startpos]
                        (Vertex v, Item& itm)
                {
                    auto d = v - iv;
                    d += startpos;
                    itm.translation(d);
                    return _objfunc(itm);
                };

                auto getNfpPoint = [&ecache](const Optimum& opt)
                {
                    return opt.hidx < 0? ecache[opt.nfpidx].coords(opt.relpos) :
                            ecache[opt.nfpidx].coords(opt.hidx, opt.relpos);
                };

                auto alignment = config_.alignment;

                auto boundaryCheck = [alignment, &merged_pile, &getNfpPoint,
                        &item, &bin, &iv, &startpos] (const Optimum& o)
                {
                    auto v = getNfpPoint(o);
                    auto d = v - iv;
                    d += startpos;
                    item.translation(d);

                    merged_pile.emplace_back(item.transformedShape());
                    auto chull = sl::convexHull(merged_pile);
                    merged_pile.pop_back();

                    double miss = 0;
                    if(alignment == Config::Alignment::DONT_ALIGN)
                       miss = sl::isInside(chull, bin) ? -1.0 : 1.0;
                    else miss = overfit(chull, bin);

                    return miss;
                };

                Optimum optimum(0, 0);
                double best_score = std::numeric_limits<double>::max();
                std::launch policy = std::launch::deferred;
                if(config_.parallel) policy |= std::launch::async;

                if(config_.before_packing)
                    config_.before_packing(merged_pile, items_, remlist);

                using OptResult = opt::Result<double>;
                using OptResults = std::vector<OptResult>;

                // Local optimization with the four polygon corners as
                // starting points
                for(unsigned ch = 0; ch < ecache.size(); ch++) {
                    auto& cache = ecache[ch];

                    OptResults results(cache.corners().size());

                    auto& rofn = rawobjfunc;
                    auto& nfpoint = getNfpPoint;
                    float accuracy = config_.accuracy;

                    __parallel::enumerate(
                                cache.corners().begin(),
                                cache.corners().end(),
                                [&results, &item, &rofn, &nfpoint, ch, accuracy]
                                (double pos, size_t n)
                    {
                        Optimizer solver(accuracy);

                        Item itemcpy = item;
                        auto contour_ofn = [&rofn, &nfpoint, ch, &itemcpy]
                                (double relpos)
                        {
                            Optimum op(relpos, ch);
                            return rofn(nfpoint(op), itemcpy);
                        };

                        try {
                            results[n] = solver.optimize_min(contour_ofn,
                                            opt::initvals<double>(pos),
                                            opt::bound<double>(0, 1.0)
                                            );
                        } catch(std::exception& e) {
                            derr() << "ERROR: " << e.what() << "\n";
                        }
                    }, policy);

                    auto resultcomp =
                            []( const OptResult& r1, const OptResult& r2 ) {
                        return r1.score < r2.score;
                    };

                    auto mr = *std::min_element(results.begin(), results.end(),
                                                resultcomp);

                    if(mr.score < best_score) {
                        Optimum o(std::get<0>(mr.optimum), ch, -1);
                        double miss = boundaryCheck(o);
                        if(miss <= 0) {
                            best_score = mr.score;
                            optimum = o;
                        } else {
                            best_overfit = std::min(miss, best_overfit);
                        }
                    }

                    for(unsigned hidx = 0; hidx < cache.holeCount(); ++hidx) {
                        results.clear();
                        results.resize(cache.corners(hidx).size());

                        // TODO : use parallel for
                        __parallel::enumerate(cache.corners(hidx).begin(),
                                      cache.corners(hidx).end(),
                                      [&results, &item, &nfpoint,
                                       &rofn, ch, hidx, accuracy]
                                      (double pos, size_t n)
                        {
                            Optimizer solver(accuracy);

                            Item itmcpy = item;
                            auto hole_ofn =
                                    [&rofn, &nfpoint, ch, hidx, &itmcpy]
                                    (double pos)
                            {
                                Optimum opt(pos, ch, hidx);
                                return rofn(nfpoint(opt), itmcpy);
                            };

                            try {
                                results[n] = solver.optimize_min(hole_ofn,
                                                opt::initvals<double>(pos),
                                                opt::bound<double>(0, 1.0)
                                                );

                            } catch(std::exception& e) {
                                derr() << "ERROR: " << e.what() << "\n";
                            }
                        }, policy);

                        auto hmr = *std::min_element(results.begin(),
                                                    results.end(),
                                                    resultcomp);

                        if(hmr.score < best_score) {
                            Optimum o(std::get<0>(hmr.optimum),
                                      ch, hidx);
                            double miss = boundaryCheck(o);
                            if(miss <= 0.0) {
                                best_score = hmr.score;
                                optimum = o;
                            } else {
                                best_overfit = std::min(miss, best_overfit);
                            }
                        }
                    }
                }

                if( best_score < global_score ) {
                    auto d = getNfpPoint(optimum) - iv;
                    d += startpos;
                    final_tr = d;
                    final_rot = initial_rot + rot;
                    can_pack = true;
                    global_score = best_score;
                }
            }

            item.translation(final_tr);
            item.rotation(final_rot);
        }

        if(can_pack) {
            ret = PackResult(item);
        } else {
            ret = PackResult(best_overfit);
        }

        return ret;
    }

    inline void finalAlign(const RawShape& pbin) {
        auto bbin = sl::boundingBox(pbin);
        finalAlign(bbin);
    }

    inline void finalAlign(_Circle<TPoint<RawShape>> cbin) {
        if(items_.empty() ||
                config_.alignment == Config::Alignment::DONT_ALIGN) return;

        nfp::Shapes<RawShape> m;
        m.reserve(items_.size());
        for(Item& item : items_) m.emplace_back(item.transformedShape());

        auto c = boundingCircle(sl::convexHull(m));

        auto d = cbin.center() - c.center();
        for(Item& item : items_) item.translate(d);
    }

    inline void finalAlign(Box bbin) {
        if(items_.empty() ||
                config_.alignment == Config::Alignment::DONT_ALIGN) return;

        nfp::Shapes<RawShape> m;
        m.reserve(items_.size());
        for(Item& item : items_) m.emplace_back(item.transformedShape());
        auto&& bb = sl::boundingBox(m);

        Vertex ci, cb;

        switch(config_.alignment) {
        case Config::Alignment::CENTER: {
            ci = bb.center();
            cb = bbin.center();
            break;
        }
        case Config::Alignment::BOTTOM_LEFT: {
            ci = bb.minCorner();
            cb = bbin.minCorner();
            break;
        }
        case Config::Alignment::BOTTOM_RIGHT: {
            ci = {getX(bb.maxCorner()), getY(bb.minCorner())};
            cb = {getX(bbin.maxCorner()), getY(bbin.minCorner())};
            break;
        }
        case Config::Alignment::TOP_LEFT: {
            ci = {getX(bb.minCorner()), getY(bb.maxCorner())};
            cb = {getX(bbin.minCorner()), getY(bbin.maxCorner())};
            break;
        }
        case Config::Alignment::TOP_RIGHT: {
            ci = bb.maxCorner();
            cb = bbin.maxCorner();
            break;
        }
        default: ; // DONT_ALIGN
        }

        auto d = cb - ci;
        for(Item& item : items_) item.translate(d);
    }

    void setInitialPosition(Item& item) {
        auto sh = item.rawShape();
        sl::translate(sh, item.translation());
        sl::rotate(sh, item.rotation());
        
        Box bb = sl::boundingBox(sh);
        
        Vertex ci, cb;
        auto bbin = sl::boundingBox(bin_);

        switch(config_.starting_point) {
        case Config::Alignment::CENTER: {
            ci = bb.center();
            cb = bbin.center();
            break;
        }
        case Config::Alignment::BOTTOM_LEFT: {
            ci = bb.minCorner();
            cb = bbin.minCorner();
            break;
        }
        case Config::Alignment::BOTTOM_RIGHT: {
            ci = {getX(bb.maxCorner()), getY(bb.minCorner())};
            cb = {getX(bbin.maxCorner()), getY(bbin.minCorner())};
            break;
        }
        case Config::Alignment::TOP_LEFT: {
            ci = {getX(bb.minCorner()), getY(bb.maxCorner())};
            cb = {getX(bbin.minCorner()), getY(bbin.maxCorner())};
            break;
        }
        case Config::Alignment::TOP_RIGHT: {
            ci = bb.maxCorner();
            cb = bbin.maxCorner();
            break;
        }
        default:;
        }

        auto d = cb - ci;
        item.translate(d);
    }

    void placeOutsideOfBin(Item& item) {
        auto&& bb = item.boundingBox();
        Box binbb = sl::boundingBox(bin_);

        Vertex v = { getX(bb.maxCorner()), getY(bb.minCorner()) };

        Coord dx = getX(binbb.maxCorner()) - getX(v);
        Coord dy = getY(binbb.maxCorner()) - getY(v);

        item.translate({dx, dy});
    }

};


}
}

#endif // NOFITPOLY_H
