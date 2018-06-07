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
class NloptOptimizer {

    nlopt::opt opt_;
    std::vector<double> lower_bounds_;
    std::vector<double> upper_bounds_;
    std::vector<double> initvals_;
    nlopt::algorithm alg_;

    enum class OptDir{
        MIN,
        MAX
    } dir_;

    template<int N, class T> struct BoundsFunc {
        void operator()(T& bounds, NloptOptimizer& self)
        {
            self.lower_bounds_[N] = bounds.min();
            self.upper_bounds_[N] = bounds.max();
        }
    };

    template<int N, class T> struct InitValFunc {
        void operator()(T& initval, NloptOptimizer& self)
        {
            self.initvals_[N] = initval;
        }
    };

    template<int N, class T> struct ResultCopyFunc {
        void operator()(T& resultval, NloptOptimizer& self)
        {
            resultval = self.initvals_[N];
        }
    };

    template<int N, class T> struct FunvalCopyFunc {
        void operator()(T& resultval, const std::vector<double>& params)
        {
            resultval = params[N];
        }
    };

    template<class Fn, class...Args>
    static double optfunc(const std::vector<double>& params,
                          std::vector<double>& grad,
                          void *data)
    {
        auto fnptr = static_cast<Fn*>(data);
        auto funval = std::tuple<Args...>();

        // copy the obtained objectfunction arguments to the funval tuple.
        metaloop::map_with_data<FunvalCopyFunc>(params, funval);

        auto ret = (*fnptr)(funval);

        return ret;
    }

    template<class...Args, class Func>
    Result<Args...> optimize(Bound<Args>... args,
                             std::tuple<Args...> initvals,
                             Func&& func)
    {
        lower_bounds_.resize(sizeof...(Args));
        upper_bounds_.resize(sizeof...(Args));
        initvals_.resize(sizeof...(Args));

        opt_ = nlopt::opt( alg_, sizeof...(Args) );

        // Copy the bounds which is obtained as a parameter pack in args into
        // lower_bounds_ and upper_bounds_
        metaloop::map_with_data<BoundsFunc>(*this, args...);

        opt_.set_lower_bounds(lower_bounds_);
        opt_.set_upper_bounds(upper_bounds_);
        opt_.set_ftol_rel(0.01);

        // Take care of the initial values, copy them to initvals_
        metaloop::map_with_data<InitValFunc>(*this, initvals);

        switch(dir_) {
        case OptDir::MIN:
            opt_.set_min_objective(optfunc<Func, Args...>, &func); break;
        case OptDir::MAX:
            opt_.set_max_objective(optfunc<Func, Args...>, &func); break;
        }

        Result<Args...> result;

        auto rescode = opt_.optimize(initvals_, result.score);
        result.resultcode = static_cast<ResultCodes>(rescode);

        metaloop::map_with_data<ResultCopyFunc>(*this, result.optimum);

        return result;
    }

public:

    inline explicit NloptOptimizer(nlopt::algorithm alg): alg_(alg) {}

    template<class...Args, class Func>
    inline Result<Args...> optimize_min(Bound<Args>... bounds,
                                        std::tuple<Args...> initvals,
                                        Func&& objectfunction)
    {
        dir_ = OptDir::MIN;
        return optimize<Args...>(bounds...,
                                 initvals,
                                 std::forward<Func>(objectfunction));
    }

    template<class...Args, class Func>
    inline Result<Args...> optimize_max(Bound<Args>... bounds,
                                        std::tuple<Args...> initvals,
                                        Func&& objectfunction)
    {
        dir_ = OptDir::MAX;
        return optimize<Args...>(bounds...,
                                 initvals,
                                 std::forward<Func>(objectfunction));
    }
};

}
}


#endif // NLOPT_BOILERPLATE_HPP
