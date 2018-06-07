#ifndef SIMPLEX_HPP
#define SIMPLEX_HPP

#include "nlopt_boilerplate.hpp"

namespace libnest2d { namespace opt {

class SimplexOptimizer: public NloptOptimizer {
public:
    inline explicit SimplexOptimizer(const StopCriteria& scr = {}):
        NloptOptimizer(nlopt::LN_NELDERMEAD, scr) {}
};

template<>
struct OptimizerSubclass<Method::SIMPLEX> { using Type = SimplexOptimizer; };

}
}

#endif // SIMPLEX_HPP
