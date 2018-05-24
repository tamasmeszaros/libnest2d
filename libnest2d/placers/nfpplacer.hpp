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

    // Store a merged polygon of all the packed items
    RawShape merged_;

public:

    inline explicit _NofitPolyPlacer(const BinType& bin): Base(bin) {}

    PackResult trypack(Item& item) {

        PackResult ret;

        bool can_pack = false;

        if(items_.empty()) {
            setInitialPosition(item);
        } else {

            // place the new item outside of the print bed to make sure it is
            // disjuct from the current merged pile
            placeOutsideOfBin(item);

            auto trsh = item.transformedShape();
            RawShape nfp = Nfp::noFitPolygon(merged_, trsh);

            double min_area = std::numeric_limits<double>::max();
            Vertex tr = {0, 0};

            auto iv = Nfp::referenceVertex(trsh);

            // place item on each the edge of this nfp
            ShapeLike::foreachContourVertex(nfp, [&]
                                            (Vertex& v)
            {
                Coord dx = getX(v) - getX(iv);
                Coord dy = getY(v) - getY(iv);

                Item placeditem(trsh);
                placeditem.translate(Vertex(dx, dy));

                if( (can_pack = placeditem.isInside(bin_)) ) {
                    auto&& m = Nfp::merge(merged_,
                                          placeditem.transformedShape());

                    double a = ShapeLike::area(ShapeLike::convexHull(m));

                    if(a < min_area) {
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

    bool pack(Item& item) {
        auto&& r = trypack(item);
        if(r) accept(r);
        return r;
    }

    inline void accept(PackResult& r) {
        if(r) {
            r.item_ptr_->translation(r.move_);
            r.item_ptr_->rotation(r.rot_);
            merged_ = items_.empty()? r.item_ptr_->transformedShape() :
                        Nfp::merge(merged_, r.item_ptr_->transformedShape());
            items_.push_back(*(r.item_ptr_));
        }
    }

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

}
}

#endif // NOFITPOLY_H
