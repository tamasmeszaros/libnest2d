#ifndef CENTERING_HPP
#define CENTERING_HPP

#include "placer_boilerplate.hpp"

namespace binpack2d { namespace strategies {

template<class RawShape>
class _CenteringPlacer: public PlacerBoilerplate<_CenteringPlacer<RawShape>,
        RawShape, _Box<TPoint<RawShape>>> {

    using Base = PlacerBoilerplate<_CenteringPlacer<RawShape>,
    RawShape, _Box<TPoint<RawShape>>>;

    DECLARE_PLACER(Base)

public:

    inline explicit _CenteringPlacer(const BinType& bin): Base(bin) {}

    PackResult trypack(Item& item) {


        return PackResult();
    }

};

}
}

#endif // CENTERING_HPP
