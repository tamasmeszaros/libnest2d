#ifndef SIMPLEX_HPP
#define SIMPLEX_HPP

#include "nlopt_boilerplate.hpp"

namespace libnest2d { namespace opt {

template<>
class Optimizer<Method::SIMPLEX>: public NloptOptimizer {
public:

    inline Optimizer(): NloptOptimizer(nlopt::LN_NELDERMEAD) {}
};

}
}

#endif // SIMPLEX_HPP
