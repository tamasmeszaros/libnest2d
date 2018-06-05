#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include <tuple>
#include <functional>
#include "common.hpp"

namespace libnest2d {

template<int N>
using Int = std::integral_constant<int, N>;

template<class T>
class OptParam {
    T min_;
    T max_;
public:
    OptParam(T min, T max): min_(min), max_(max) {}
    inline const T min() const BP2D_NOEXCEPT { return min_; }
    inline const T max() const BP2D_NOEXCEPT { return max_; }
};

class Optimizer {

    template<class Opt, class V> void processArg(const Opt& opt, V& val)
    {
        std::cout << opt.min() << " " << opt.max()
                  << std::endl;
        val = 0;
    }

    template <typename U, class Opts>
    class ArgProcessor {};

    template <class Opts>
    class ArgProcessor<Int<0>, Opts> {
        Optimizer& opt_;
    public:
        template<class...Args>
        void process(const Opts& opts, std::tuple<Args...>& valtup) {
            opt_.processArg(std::get<0>(opts), std::get<0>(valtup));
        }

        ArgProcessor(Optimizer& opt): opt_(opt) {}
    };

    template <int N, class Opts>
    class ArgProcessor<Int<N>, Opts> {
        Optimizer& opt_;
    public:
        template<class...Args>
        void process(const Opts& opts, std::tuple<Args...>& valtup) {
            opt_.processArg(std::get<N>(opts), std::get<N>(valtup));
            ArgProcessor<Int<N-1>, Opts> p(opt_);
            p.process(opts, valtup);
        }

        ArgProcessor(Optimizer& opt): opt_(opt) {}
    };

public:

    template<class...Args>
    using OptFunc = std::function<double(std::tuple<Args...>)>;

    template<class...Args>
    void optimize(OptParam<Args>... args, OptFunc<Args...> func) {

        using OptTup = std::tuple<OptParam<Args>...>;
        using ValTup = std::tuple<Args...>;

        std::tuple<OptParam<Args>...> params(args...);
        ValTup values;

        ArgProcessor<Int<sizeof...(Args)-1>, OptTup> argProcessor(*this);
        argProcessor.process(params, values);

        func(values);
    }

private:

};




}

#endif // OPTIMIZER_HPP
