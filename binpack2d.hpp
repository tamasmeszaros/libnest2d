#ifndef BINPACK2D_HPP
#define BINPACK2D_HPP

#include <memory>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>

#include "geometries.hpp"

namespace binpack2d {

const auto BP2D_CONSTEXPR Pi = 2*acos(0);

class Double {
  double val_;
public:
  Double(): val_(double{}) { }
  Double(double d) : val_(d) { }

  operator double() const BP2D_NOEXCEPT { return val_; }
  operator double&() BP2D_NOEXCEPT { return val_; }
};

class Degrees;

class Radians: public Double {
public:

    Radians(double rads = Double() ): Double(rads) {}
    Radians(const Degrees& degs);

    operator Degrees();
};

class Degrees: public Double {
public:
    Degrees(double deg = Double()): Double(deg) {}
    Degrees(const Radians& rads): Double( rads * 180/Pi ) {}
};

inline bool operator==(const Degrees& deg, const Radians& rads) {
    Degrees deg2 = rads;
    auto diff = abs(deg - deg2);
    return diff < 0.0001;
}

inline bool operator==(const Radians& rads, const Degrees& deg) {
    return deg == rads;
}

template<class RawShape,
         class TTransformation = TTransformation<RawShape> >
class _Item {

    using TShape = _Shape<RawShape>;

    TShape shape_;

    using Unit = TCoord<RawShape>;

    TTransformation tr_;

public:

    _Item(const TShape& shape): shape_(shape) {}
    _Item(TShape&& shape): shape_(std::move(shape)) {}

//    _Item(const _Rectangle<RawShape>& rect):shape_(rect) {

//    }
//    _Item& operator=(TShape&& shape) {
//        shape_ = std::move(shape);
//    }

//    _Item& operator=(_Rectangle<RawShape>&& shape) {
//        shape_ = std::move(shape);
//    }

    template<class...Args> _Item(Args...args): shape_(args...) {}

    TShape transformedShape() {
        TShape sh(shape_);
        sh.transform(tr_);
        return sh;
    }

    void offset(Unit x, Unit y) { tr_.get(0, 0) += x; tr_.get(1, 1) += y; }

    void rotate(const Radians& /*rads*/) {
//        static_assert(false, "unimplemented");
    }

    double area() const { return shape_.area(); }

    const TShape& shape() const { return shape_; }
};


template<class RawShape, class TBinShape>
class _DummyPlacementStrategy {
    TBinShape bin_;
public:

    _DummyPlacementStrategy(const TBinShape& bin): bin_(bin) {}

    bool insertItem(_Item<RawShape>& /*item*/) { return true; }

};

template<class RawShape>
class _DummySelectionStrategy {
    using Container = typename std::vector<_Item<RawShape>>;

    Container store_;
    unsigned long pos_;

public:

    using Item = _Item<RawShape>;
    using ItemRef = typename std::reference_wrapper<Item>;
    using ItemGroup = typename std::vector<ItemRef>;

    template<class TBin>
    using PlacementStrategy = _DummyPlacementStrategy<RawShape, TBin>;

    template<class TIterator>
    void addItems(TIterator first, TIterator last) {

        store_.clear();
        store_.reserve(last-first);

        std::copy(first, last, std::back_inserter(store_));

        pos_ = 0;

        auto sortfunc = [](Item& i1, Item& i2) {
            return i1.area() < i2.area();
        };

        std::sort(store_.begin(), store_.end(), sortfunc);
    }


    ItemGroup nextGroup() {
        return ItemGroup({store_[ pos_++ ]});
    }

};



template<class TShape,
         class SelectionStrategy = _DummySelectionStrategy<TShape>>
class _Arranger {

    SelectionStrategy sel_strategy_;

public:

    _Arranger() {}

    using Unit = TCoord< TPoint<TShape> >;

    struct Config {
        Unit minObjectDistance;
    };

    template<class TIterator>
    void arrange(TIterator from,
                 TIterator to,
                 const _Rectangle<TShape>& bin,
                 Config config = Config()) {

        sel_strategy_.addItems(from, to);

        auto placer = SelectionStrategy::template PlacementStrategy<decltype(bin)>(bin);

    }

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

#endif // BINPACK2D_HPP
