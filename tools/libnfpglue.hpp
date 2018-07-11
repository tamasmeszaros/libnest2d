#ifndef LIBNFPGLUE_HPP
#define LIBNFPGLUE_HPP

#include <libnest2d/clipper_backend/clipper_backend.hpp>

namespace libnest2d {

PolygonImpl _nfp(const PolygonImpl& sh, const PolygonImpl& cother);

template<>
struct Nfp::NfpImpl<PolygonImpl, NfpLevel::CONVEX_ONLY> {
    PolygonImpl operator()(const PolygonImpl& sh, const PolygonImpl& cother);
};

template<>
struct Nfp::NfpImpl<PolygonImpl, NfpLevel::ONE_CONVEX> {
    PolygonImpl operator()(const PolygonImpl& sh, const PolygonImpl& cother);
};

template<>
struct Nfp::NfpImpl<PolygonImpl, NfpLevel::BOTH_CONCAVE> {
    PolygonImpl operator()(const PolygonImpl& sh, const PolygonImpl& cother);
};

template<>
struct Nfp::NfpImpl<PolygonImpl, NfpLevel::BOTH_CONCAVE_WITH_HOLES> {
    PolygonImpl operator()(const PolygonImpl& sh, const PolygonImpl& cother);
};



}


#endif // LIBNFPGLUE_HPP
