#ifndef BINPACK2D_H
#define BINPACK2D_H

#include <memory>
#include <vector>

#include "geometries.hpp"

namespace binpack2d {

class Item {
    Shape shape_;

public:

    void translate();
    void rotate();

};

using _Container = std::vector<Item>;

class ItemStore: _Container {
public:
    using _Container::vector<Item>;

    using _Container::begin;
    using _Container::cbegin;
    using _Container::end;
    using _Container::cend;

    using _Container::push_back;
    using _Container::size;

};

class Packager {

    class Impl; std::unique_ptr<Impl> impl_;

public:

    Packager();
    ~Packager();

    struct Config {
        unsigned long minObjectDistance;
    };

    void arrange(ItemStore& items,
                 const Rectangle& bin,
                 const Config& config = {0} );

    void arrange(ItemStore& items,
                 const Ellipse& bin,
                 const Config& config = {0} );

    template<class TBin, class...Args> void arrange(ItemStore& items,
                                        const TBin& bin,
                                        Args...args)
    {
        arrange(items, bin, Config{args...});
    }

};

}

#endif // BINPACK2D_H
