#ifndef NLOPT_BOILERPLATE_HPP
#define NLOPT_BOILERPLATE_HPP

#include <nlopt.hpp>
#include <libnest2d/optimizer.hpp>

namespace libnest2d { namespace opt {

/**
 * Optimizer based on NLopt.
 *
 * All the optimized types have to be convertible to double.
 */
class NloptOptimizer: public Optimizer<NloptOptimizer> {
    nlopt::opt opt_;
    std::vector<double> lower_bounds_;
    std::vector<double> upper_bounds_;
    std::vector<double> initvals_;
    nlopt::algorithm alg_;

    using Base = Optimizer<NloptOptimizer>;

    friend Base;

    // ********************************************************************** */

    // TODO: CHANGE FOR LAMBDAS WHEN WE WILL MOVE TO C++14

    struct BoundsFunc {
        NloptOptimizer& self;
        inline explicit BoundsFunc(NloptOptimizer& o): self(o) {}

        template<class T> void operator()(int N, T& bounds)
        {
            self.lower_bounds_[N] = bounds.min();
            self.upper_bounds_[N] = bounds.max();
        }
    };

    struct InitValFunc {
        NloptOptimizer& self;
        inline explicit InitValFunc(NloptOptimizer& o): self(o) {}

        template<class T> void operator()(int N, T& initval)
        {
            self.initvals_[N] = initval;
        }
    };

    struct ResultCopyFunc {
        NloptOptimizer& self;
        inline explicit ResultCopyFunc(NloptOptimizer& o): self(o) {}

        template<class T> void operator()(int N, T& resultval)
        {
            resultval = self.initvals_[N];
        }
    };

    struct FunvalCopyFunc {
        using D = const std::vector<double>;
        D& params;
        inline explicit FunvalCopyFunc(D& p): params(p) {}

        template<class T> void operator()(int N, T& resultval)
        {
            resultval = params[N];
        }
    };

    /* ********************************************************************** */

    template<class Fn, class...Args>
    static double optfunc(const std::vector<double>& params,
                          std::vector<double>& grad,
                          void *data)
    {
        auto fnptr = static_cast<Fn*>(data);
        auto funval = std::tuple<Args...>();

        // copy the obtained objectfunction arguments to the funval tuple.
        metaloop::map(FunvalCopyFunc(params), funval);

        auto ret = (*fnptr)(funval);

        return ret;
    }

    template<class Func, class...Args>
    Result<Args...> optimize(Func&& func,
                             std::tuple<Args...> initvals,
                             Bound<Args>... args)
    {
        lower_bounds_.resize(sizeof...(Args));
        upper_bounds_.resize(sizeof...(Args));
        initvals_.resize(sizeof...(Args));

        opt_ = nlopt::opt(alg_, sizeof...(Args) );

        // Copy the bounds which is obtained as a parameter pack in args into
        // lower_bounds_ and upper_bounds_
        metaloop::map(BoundsFunc(*this), args...);

        opt_.set_lower_bounds(lower_bounds_);
        opt_.set_upper_bounds(upper_bounds_);

        switch(this->stopcr_.type) {
        case StopLimitType::ABSOLUTE:
            opt_.set_ftol_abs(stopcr_.stoplimit); break;
        case StopLimitType::RELATIVE:
            opt_.set_ftol_rel(0.01); break;
        }

        // Take care of the initial values, copy them to initvals_
        metaloop::map(InitValFunc(*this), initvals);

        switch(dir_) {
        case OptDir::MIN:
            opt_.set_min_objective(optfunc<Func, Args...>, &func); break;
        case OptDir::MAX:
            opt_.set_max_objective(optfunc<Func, Args...>, &func); break;
        }

        Result<Args...> result;

        auto rescode = opt_.optimize(initvals_, result.score);
        result.resultcode = static_cast<ResultCodes>(rescode);

        metaloop::map(ResultCopyFunc(*this), result.optimum);

        return result;
    }

public:
    inline explicit NloptOptimizer(nlopt::algorithm alg,
                                   StopCriteria stopcr = {}):
        Base(stopcr), alg_(alg) {}

};

}
}


#endif // NLOPT_BOILERPLATE_HPP
