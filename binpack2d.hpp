#ifndef BINPACK2D_H
#define BINPACK2D_H

#include <memory>
#include <vector>

#include "geometries.hpp"

namespace binpack2d {

template<class TShape>
class DummyCoding {
public:
    using Container = typename std::vector<TShape>;
};

template<class TShape, class Coding= DummyCoding<TShape> >
class Arranger {
    typename Coding::Container store_;

public:
    using Unit = typename TCoord< TPoint<TShape> >;

    struct Config {
        Unit minObjectDistance;
    };

    template<class TIterator>
    void arrange(TIterator from,
                 TIterator to,
                 const Rectangle& bin,
                 Config config = Config());

    template<class TIterator, class TBin, class...Args>
    void arrange(TIterator from,
                 TIterator to,
                 const TBin& bin,
                 Args...args)
    {
        arrange(from, to, bin, Config{args...});
    }

};

}

#endif // BINPACK2D_H
