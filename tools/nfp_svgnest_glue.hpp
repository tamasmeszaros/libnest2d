#ifndef NFP_SVGNEST_GLUE_HPP
#define NFP_SVGNEST_GLUE_HPP

#include "nfp_svgnest.hpp"

#include <libnest2d/clipper_backend/clipper_backend.hpp>

namespace libnest2d {

namespace __svgnest {

template<> struct _Tol<TCoord<PointImpl>> {
    static const BP2D_CONSTEXPR TCoord<PointImpl> Value = 1000000;
};

}

namespace nfp {

using NfpR = NfpResult<PolygonImpl>;

template<> struct NfpImpl<PolygonImpl, NfpLevel::ONE_CONVEX> {
    NfpR operator()(const PolygonImpl& sh, const PolygonImpl& cother) {
        return nfpConvexOnly(sh, cother);
    }
};

template<>
struct NfpImpl<PolygonImpl, NfpLevel::BOTH_CONCAVE> {
    NfpR operator()(const PolygonImpl& sh, const PolygonImpl& cother) {
        namespace sl = shapelike;
        using alg = __svgnest::_alg<PolygonImpl>;

        auto nfp_p = alg::noFitPolygon(sl::getContour(sh),
                                       sl::getContour(cother), true, false);

        return {nfp_p, referenceVertex(nfp_p)};
    }
};

template<> struct MaxNfpLevel<PolygonImpl> {
    static const BP2D_CONSTEXPR NfpLevel value = NfpLevel::BOTH_CONCAVE;
};

}}

#endif // NFP_SVGNEST_GLUE_HPP
