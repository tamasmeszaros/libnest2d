#ifndef PARTICLESWARM_HPP
#define PARTICLESWARM_HPP

#include <libnest2d/optimizer.hpp>

namespace libnest2d { namespace opt {

class ParticleswarmOptimizer {
public:
    inline explicit ParticleswarmOptimizer(const StopCriteria& scr = {}) {}

    inline ParticleswarmOptimizer& localMethod(Method m) {
        localmethod_ = m;
        return *this;
    }

    inline void seed(unsigned long val) { nlopt::srand(val); }
};

template<>
struct OptimizerSubclass<Method::G_PARTICLESWARM> { using Type = ParticleswarmOptimizer; };

template<>
inline TOptimizer<Method::G_PARTICLESWARM> GlobalOptimizer<Method::G_PARTICLESWARM>(
        Method localm, const StopCriteria& scr )
{
    return ParticleswarmOptimizer (scr).localMethod(localm);
}

}
}

#endif // GENETIC_HPP
