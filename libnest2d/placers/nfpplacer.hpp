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

    bool allow_rotations = false;
    Alignment alignment;
};

template<class RawShape>
class _NofitPolyPlacer: public PlacerBoilerplate<_NofitPolyPlacer<RawShape>,
        RawShape, _Box<TPoint<RawShape>>, NfpPConfig<RawShape>> {

    using Base = PlacerBoilerplate<_NofitPolyPlacer<RawShape>,
    RawShape, _Box<TPoint<RawShape>>, NfpPConfig<RawShape>>;

    DECLARE_PLACER(Base)

    using Box = _Box<TPoint<RawShape>>;

    static std::vector<_Item<RawShape>> dbg_items_;

public:

    inline explicit _NofitPolyPlacer(const BinType& bin): Base(bin) {}

    PackResult trypack(Item& item) {

        PackResult ret;

        bool can_pack = false;

        if(items_.empty()) {
            setInitialPosition(item);
            can_pack = item.isInside(bin_);
        } else {

            // place the new item outside of the print bed to make sure it is
            // disjuct from the current merged pile
            placeOutsideOfBin(item);

            auto trsh = item.transformedShape();
            Nfp::Shapes<RawShape> nfps;
            for(Item& sh : items_) {
                nfps = Nfp::merge(nfps, Nfp::noFitPolygon(sh.transformedShape(),
                                                          trsh));
            }

//            for(Item& sh : items_) {
//                nfps.emplace_back(sh.transformedShape());
//            }
//            auto hull = ShapeLike::convexHull(nfps);
//            nfps = Nfp::noFitPolygon(nfps, trsh);
//            auto nfp = Nfp::noFitPolygon(hull, trsh);

//            if(items_.size() > 1) {
//            for(auto& nfp : nfps) dbg_items_.emplace_back(nfp);
//            dbg_items_.emplace_back(hull);
//            }

            double min_area = std::numeric_limits<double>::max();
            Vertex tr = {0, 0};

            auto iv = Nfp::referenceVertex(trsh);

            // place item on each the edge of this nfp
            for(auto& nfp : nfps)
            ShapeLike::foreachContourVertex(nfp, [&]
                                            (Vertex& v)
            {
                Coord dx = getX(v) - getX(iv);
                Coord dy = getY(v) - getY(iv);

                Item placeditem(trsh);
                placeditem.translate(Vertex(dx, dy));

                if( placeditem.isInside(bin_) ) {
                    Nfp::Shapes<RawShape> m;
                    m.reserve(items_.size());

                    for(Item& pi : items_)
                        m.emplace_back(pi.transformedShape());
                    m.emplace_back(placeditem.transformedShape());

                    auto b = ShapeLike::boundingBox(m);

                    auto a = static_cast<double>(std::max(b.height(),
                                                          b.width()));

                    if(a < min_area) {
                        can_pack = true;
                        min_area = a;
                        tr = {dx, dy};
                    }
                }
            });

            item.translate(tr);
        }

        if(can_pack) {
            ret = PackResult(item);
        }

        return ret;
    }

#ifndef NDEBUG
    inline typename Base::ItemGroup getItems() {
        items_.insert(items_.end(), dbg_items_.begin(), dbg_items_.end());
        return items_;
    }
#endif

private:

    void setInitialPosition(Item& item) {

        switch(config_.alignment) {
        case Config::Alignment::CENTER: {
            Box&& bb = item.boundingBox();

            Vertex&& ci = bb.center();
            Vertex&& cb = bin_.center();
            Coord dx = getX(cb) - getX(ci);
            Coord dy = getY(cb) - getY(ci);

            item.translate({dx, dy});
        }
        default:
            ;
        }
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

template<class RawShape>
std::vector<_Item<RawShape>> _NofitPolyPlacer<RawShape>::dbg_items_ = {};


}
}

#endif // NOFITPOLY_H
