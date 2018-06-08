#ifndef GENETIC_HPP
#define GENETIC_HPP

#include "nlopt_boilerplate.hpp"

namespace libnest2d { namespace opt {

class GeneticOptimizer: public NloptOptimizer {
public:
    inline explicit GeneticOptimizer(const StopCriteria& scr = {}):
        NloptOptimizer(method2nloptAlg(Method::GENETIC), scr) {}

    inline GeneticOptimizer& localMethod(Method m) {
        localmethod_ = m;
        return *this;
    }
};

template<>
struct OptimizerSubclass<Method::GENETIC> { using Type = GeneticOptimizer; };

template<> TOptimizer<Method::GENETIC> GlobalOptimizer<Method::GENETIC>(
        Method localm, const StopCriteria& scr )
{
    return GeneticOptimizer (scr).localMethod(localm);
}

}
}

#endif // GENETIC_HPP
