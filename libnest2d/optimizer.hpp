#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <tuple>
#include <functional>
#include <type_traits>
#include "common.hpp"

namespace libnest2d { namespace opt {

using std::forward;
using std::tuple;
using std::get;
using std::tuple_element;

template<class T>
class Bound {
    T min_;
    T max_;
public:
    Bound(T min, T max): min_(min), max_(max) {}
    inline const T min() const BP2D_NOEXCEPT { return min_; }
    inline const T max() const BP2D_NOEXCEPT { return max_; }
};

class metaloop {

// A helper alias to create integer values wrapped as a type. It is nessecary
// because a non type template parameter (such as int) would be prohibited in
// a partial specialization. Also for the same reason we have to use a class
// _Metaloop instead of a simple function as a functor. A function cannot be
// partially specialized in a way that is neccesary for this trick.
template<int N> using Int = std::integral_constant<int, N>;

template<int N, class Fn> class MapFn {
    Fn fn_;
public:
    inline MapFn(Fn fn): fn_(fn) {}

    template<class...Args> void operator ()(Args...args) {
        fn_(N, forward<Args>(args)...);
    }
};

/**
 * \brief Loop over a parameter pack and do something with each element.
 *
 * We create a mechanism for looping over a parameter pack in compile time.
 * \tparam Idx is the loop index which will be incremented at each recursion.
 * \tparam Fn Is a callable template class with an integer template parameter
 * as the index and another template parameter which is the index-th type of the
 * parameter pack. The function is expected to take this type as its first
 * argument. The second can be any type passed on through the run method's data
 * parameter. A suitable declaration for the template Fn would be for example:
 *
 *      template<int N, class T> struct {
 *          template<class Data> void operator()(T&& element, Data&& data);
 *      }
 *
 * The data parameter will be the same that was given be the caller of the run()
 * method of _Metaloop passed on untouched.
 *
 * \tparam Args The parameter pack that will be processed.
 *
 */
template <typename Idx, template<int, class> class Fn, class...Args>
class _MetaLoop {};

// Implementation for the last element of Args...
template <template<int, class> class Fn, class...Args>
class _MetaLoop<Int<sizeof...(Args)-1>, Fn, Args...> {
public:

    const static BP2D_CONSTEXPR int M = sizeof...(Args)-1;

    template<class Tup, class Data>
    void run( Tup&& valtup, Data&& data) {
        using TV = typename tuple_element<M, remove_ref_t<Tup>>::type;
        Fn<M, TV&> () (get<M>(valtup), forward<Data>(data));
    }
};

// Implementation for the N-th element of Args...
template <int N, template<int, class> class Fn, class...Args>
class _MetaLoop<Int<N>, Fn, Args...> {
public:

    template<class Tup, class Data>
    void run(Tup&& valtup, Data&& data) {
        using TV = typename tuple_element<N, remove_ref_t<Tup>>::type;
        Fn<N, TV&> () (get<N>(valtup), forward<Data>(data));

        // Recursive call to process the next element of Args
        _MetaLoop<Int<N+1>, Fn, Args...> ()
                .run(forward<Tup>(valtup), forward<Data>(data));
    }
};

// Instantiation: We must instantiate the template with the zero index because
// the generalized version calls the incremented instantiations recursively.
// Once the instantiation with the last index is called, the terminating version
// of run is called which does not call itself anymore. If you are annoyed, at
// least you have learned a functional programming pattern.
template<template<int, class> class Fn, class...Args>
using MetaLoop = _MetaLoop<Int<0>, Fn, Args...>;

public:

/**
 * \brief The final usable function template.
 *
 * This is similar to what varags was on C but in compile time C++11.
 * You can call:
 * map<`Function template name`>(<arbitrary number of arguments of any type>,
 *                               <data for the mapping function>);
 * For example:
 *
 *      template<int N, class T> struct mapfunc {
 *          void operator()(T&& element, const std::string& message) {
 *              std::cout << message << " " << N <<": " << element << std::endl;
 *          }
 *      };
 *
 *      map_with_data<mapfunc>("The value of the parameter", 'a', 10, 151.545);
 *
 * This yields the output:
 * The value of the parameter 0: a
 * The value of the parameter 1: 10
 * The value of the parameter 2: 151.545
 *
 * As an addition, the function can be called with a tuple holding the
 * arguments insted of a parameter pack.
 *
 */
template<template<int, class> class Fn, class Data, class...Args>
inline static void map_with_data(Data&& data, Args&&...args) {
    MetaLoop<Fn, Args...>().run(tuple<Args&&...>(forward<Args>(args)...),
                                forward<Data>(data));
}

template<template<int, class> class Fn, class Data, class...Args>
inline static void map_with_data(Data&& data, tuple<Args...>&& tup) {
    MetaLoop<Fn, Args...>().run(std::move(tup), forward<Data>(data));
}

template<template<int, class> class Fn, class Data, class...Args>
inline static void map_with_data(Data&& data, tuple<Args...>& tup) {
    MetaLoop<Fn, Args...>().run(tup, forward<Data>(data));
}

template<template<int, class> class Fn, class Data, class...Args>
inline static void map_with_data(Data&& data, const tuple<Args...>& tup) {
    MetaLoop<Fn, Args...>().run(tup, forward<Data>(data));
}

/**
 * Same as the previous version but without the addtional data parameter.
 * Again it can be called with a tuple as well instead of a parameter pack.
 *
 * Remember that the template function object still has to take two parameters.
 * The second one can be a void* i nthis case.
 */
template<template<int, class> class Fn, class...Args>
inline static void map(Args&&...args) {
    MetaLoop<Fn, Args...>().run(tuple<Args&&...>(forward<Args>(args)...),
                                nullptr);
}

template<template<int, class> class Fn, class...Args>
inline static void map(tuple<Args...>&& tup) {
    MetaLoop<Fn, Args...>().run(std::move(tup), nullptr);
}

template<template<int, class> class Fn, class...Args>
inline static void map(tuple<Args...>& tup) {
    MetaLoop<Fn, Args...>().run(tup, nullptr);
}

template<template<int, class> class Fn, class...Args>
inline static void map(const tuple<Args...>& tup) {
    MetaLoop<Fn, Args...>().run(tup, nullptr);
}

};

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

enum class StopLimitType {
    ABSOLUTE,
    RELATIVE
};

struct StopCriteria {
    StopLimitType type = StopLimitType::RELATIVE;
    double stoplimit = 0.0001;
};

template<Method>
class Optimizer {

    StopCriteria stopcr_;

    template<int N, class T>
    struct ArgFunc {
        template<class Data>
        void operator()(T& val, Data&& data)
        {
            std::cout << val.min() << " " << val.max() << std::endl;
        }
    };

public:

    inline explicit Optimizer(StopCriteria scr = {}): stopcr_(scr) {}

    template<class...Args, class Func>
    inline Result<Args...> optimize_min(Bound<Args>... bounds,
                                        std::tuple<Args...> initvals,
                                        Func&& objectfunction)
    {
        metaloop::map<ArgFunc>(bounds...);
        return Result<Args...>();
    }

    template<class...Args, class Func>
    inline Result<Args...> optimize_max(Bound<Args>... bounds,
                                        std::tuple<Args...> initvals,
                                        Func&& objectfunction)
    {
        metaloop::map<ArgFunc>(bounds...);
        return Result<Args...>();
    }

};

}
}

#endif // OPTIMIZER_HPP
