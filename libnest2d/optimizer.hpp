#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <tuple>
#include <functional>
#include "common.hpp"

namespace libnest2d { namespace opt {

template<int N>
using Int = std::integral_constant<int, N>;

template<class T>
class Bound {
    T min_;
    T max_;
public:
    Bound(T min, T max): min_(min), max_(max) {}
//    Param(std::initializer_list<T> ilist):
//        min_(*ilist.begin()),
//        max_(*std::prev(ilist.end())) {}
    inline const T min() const BP2D_NOEXCEPT { return min_; }
    inline const T max() const BP2D_NOEXCEPT { return max_; }
};

namespace metaloop {

template <typename U, template<int, class> class Fn, class...Args>
class _MetaLoop {};

template <template<int, class> class Fn, class...Args>
class _MetaLoop<Int<0>, Fn, Args...> {
public:

    using TArgs = std::tuple<Args...>;

    template<class Tup, class Data>
    void run(Tup&& valtup, Data&& data) {
        using TV = typename std::tuple_element<0, TArgs>::type;
        Fn<0, TV&> fn;
        fn(std::get<0>(valtup), std::forward<Data>(data));
    }
};

template <int N, template<int, class> class Fn, class...Args>
class _MetaLoop<Int<N>, Fn, Args...> {
public:

    using TArgs = std::tuple<Args...>;

    template<class Tup, class Data>
    void run(Tup&& valtup, Data&& data) {

        static_assert(N >= 0, "Cannot have negative dimesion values!");

        using TV = typename std::tuple_element<N, TArgs>::type;
        Fn<N, TV&> fn;
        fn(std::get<N>(valtup), std::forward<Data>(data));
        _MetaLoop<Int<N-1>, Fn, Args...> p;
        p.run(std::forward<std::tuple<Args...>>(valtup),
                  std::forward<Data>(data));
    }
};

template<template<int, class> class Fn, class...Args>
using MetaLoop = _MetaLoop<Int<sizeof...(Args)-1>, Fn, Args...>;

}

enum class Method {
    SIMPLEX,
    GENETIC,
    SIMULATED_ANNEALING,
    //...
};

enum ResultCodes {
    FAILURE = -1, /* generic failure code */
    INVALID_ARGS = -2,
    OUT_OF_MEMORY = -3,
    ROUNDOFF_LIMITED = -4,
    FORCED_STOP = -5,
    SUCCESS = 1, /* generic success code */
    STOPVAL_REACHED = 2,
    FTOL_REACHED = 3,
    XTOL_REACHED = 4,
    MAXEVAL_REACHED = 5,
    MAXTIME_REACHED = 6
};

template<class...Args>
struct Result {
    ResultCodes resultcode;
    std::tuple<Args...> optimum;
    double score;
};

template<Method>
class Optimizer {

    template<int N, class T>
    struct ArgFunc {
        template<class Data>
        void operator()(T& val, Data&& data)
        {
            static_assert(N >= 0, "Cannot have negative dimesion values!");

            std::cout << val.min() << " " << val.max()
                      << std::endl;
        }
    };

public:

    // Causes compile error on Clang 6.0 if I use the std::function declaration
//    template<class...Args>
//    using OptFunc = std::function<double(std::tuple<Args...>,
//                                         std::tuple<Args...>&)>;
    template<class...Args, class Func>
    inline Result<Args...> optimize_min(Bound<Args>... bounds,
                                        std::tuple<Args...> initvals,
                                        Func&& objectfunction)
    {
        metaloop::MetaLoop<ArgFunc, Bound<Args>...> ()
                .run(std::make_tuple(bounds...), nullptr);

        return Result<Args...>();

    }

    template<class...Args, class Func>
    inline Result<Args...> optimize_max(Bound<Args>... bounds,
                                        std::tuple<Args...> initvals,
                                        Func&& objectfunction)
    {
        metaloop::MetaLoop<ArgFunc, Bound<Args>...> ()
                .run(std::make_tuple(bounds...), nullptr);

        return Result<Args...>();
    }

};

}
}

#endif // OPTIMIZER_HPP
