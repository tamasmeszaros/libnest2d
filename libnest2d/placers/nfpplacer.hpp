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
    RawShape, _Box<TPoint<RawShape>>>;

    DECLARE_PLACER(Base)

    using Box = _Box<TPoint<RawShape>>;

    // Store a merged polygon of all the packed items
    RawShape merged_;

public:

    using typename NfpPConfig<RawShape>::Alignment;

    inline explicit _NofitPolyPlacer(const BinType& bin): Base(bin) {}

    PackResult trypack(Item& item) {

        setInitialPosition(item);

        if(!item.isInside(bin_)) return PackResult();


        return PackResult();
    }

private:

    void setInitialPosition(Item& item) {

        switch(config_.alignment) {
        case Alignment::CENTER: {
            Box&& bb = item.boundingBox();

            Vertex&& ci = center(bb);
            Vertex&& cb = center(bin_);
            Coord dx = getX(cb) - getX(ci);
            Coord dy = getY(cb) - getY(ci);

            item.translate({dx, dy});
        }
        default:
            ;
        }
    }

    static inline Vertex center(const Box& box) {
        auto& minc = box.minCorner();
        auto& maxc = box.maxCorner();

        Vertex ret =  {
            static_cast<Coord>( std::round((getX(minc) + getX(maxc))/2.0) ),
            static_cast<Coord>( std::round((getY(minc) + getY(maxc))/2.0) )
        };

        return ret;
    }

};

}
}

#endif // NOFITPOLY_H
