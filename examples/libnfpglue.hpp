#ifndef LIBNFPGLUE_HPP
#define LIBNFPGLUE_HPP

#include "tools/libnfporb.hpp"
#include <libnest2d/geometries_nfp.hpp>

namespace libnest2d {

template<class RawShape>
struct Nfp::NfpImpl<RawShape, NfpLevel::ONE_CONVEX> {
    RawShape operator()(const RawShape& sh, const RawShape& cother) {
        return RawShape();
    }
};

template<class RawShape>
struct Nfp::NfpImpl<RawShape, NfpLevel::BOTH_CONCAVE> {
    RawShape operator()(const RawShape& sh, const RawShape& cother) {
        return RawShape();
    }
};


}


#endif // LIBNFPGLUE_HPP
