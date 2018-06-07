#ifndef NOFITPOLY_HPP
#define NOFITPOLY_HPP

#include "placer_boilerplate.hpp"
#include "../geometries_nfp.hpp"

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
};

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

            auto initial_tr = item.translation();
            auto initial_rot = item.rotation();
            double min_area = std::numeric_limits<double>::max();
            Vertex final_tr = {0, 0};
            Radians final_rot = 0;
            Nfp::Shapes<RawShape> nfps;

//            auto objfunc = [this](double nfppos, Radians angle){
//                return 0; // score = pack efficiency
//            };

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

                nfps.clear();
                for(Item& sh : items_) {
                    auto subnfp = Nfp::noFitPolygon(sh.transformedShape(),
                                                    trsh);
                    #ifndef NDEBUG
                    #ifdef DEBUG_EXPORT_NFP
                        this->debug_items_.emplace_back(subnfp);
                    #endif
                        auto vv = ShapeLike::isValid(sh.transformedShape());
                        assert(vv.first);

                        auto vnfp = ShapeLike::isValid(subnfp);
                        assert(vnfp.first);
                    #endif

                    nfps = Nfp::merge(nfps, subnfp);
                }

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

                        auto b = ShapeLike::convexHull(m);
                        auto a = ShapeLike::area(b);

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
