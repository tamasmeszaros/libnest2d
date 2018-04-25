#ifndef BINPACK2D_HPP
#define BINPACK2D_HPP

#include <memory>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <limits>
#include <functional>

#include "geometry_traits.hpp"

namespace binpack2d {

template<class RawShape>
class _Item {
    RawShape sh_;
    TPoint<RawShape> offset_;
    Radians rotation_;
    bool has_rotation_ = false, has_offset_ = false;

    // For caching the transformation
    mutable RawShape tr_cache_;
    mutable bool tr_cache_valid_ = false;
public:

    static BP2D_CONSTEXPR Orientation orientation() {
        return OrientationType<RawShape>::Value;
    }

    inline _Item(const RawShape& sh): sh_(sh) {}

    inline _Item(RawShape&& sh): sh_(std::move(sh)) {}

    inline _Item(const std::initializer_list< TPoint<RawShape> >& il):
        sh_(ShapeLike::create<RawShape>(il)) {}

    inline std::string toString() const { return ShapeLike::toString(sh_); }

    inline TVertexIterator<RawShape> begin() {
        return ShapeLike::begin(sh_);
    }

    inline TVertexConstIterator<RawShape> begin() const {
        return ShapeLike::cbegin(sh_);
    }

    inline TVertexConstIterator<RawShape> cbegin() const {
        return ShapeLike::cbegin(sh_);
    }

    inline TVertexIterator<RawShape> end() {
        return ShapeLike::end(sh_);
    }

    inline TVertexConstIterator<RawShape> end() const {
        return ShapeLike::cend(sh_);
    }

    inline TVertexConstIterator<RawShape> cend() const {
        return ShapeLike::cend(sh_);
    }

    inline TPoint<RawShape> vertex(unsigned long idx) const BP2D_NOEXCEPT {
        return ShapeLike::vertex(sh_, idx);
    }

    inline double area() const {
        return ShapeLike::area(sh_);
    }

    inline unsigned long vertexCount() const BP2D_NOEXCEPT {
        return ShapeLike::contourVertexCount(sh_);
    }

    inline bool isPointInside(const TPoint<RawShape>& p) {
        return ShapeLike::isInside(p, sh_);
    }

    inline bool isInside(const _Item& outer) {
        return ShapeLike::isInside(transformedShape(),
                                   outer.transformedShape());
    }

    inline void translate(const TPoint<RawShape>& d) BP2D_NOEXCEPT {
        offset_ += d; has_offset_ = true;
        tr_cache_valid_ = false;
    }

    inline void rotate(const Radians& rads) BP2D_NOEXCEPT {
        rotation_ += rads;
        has_rotation_ = true;
        tr_cache_valid_ = false;
    }

    inline RawShape transformedShape() const {
        if(tr_cache_valid_) return tr_cache_;

        RawShape cpy = sh_;
        if(has_rotation_) ShapeLike::rotate(cpy, rotation_);
        if(has_offset_) ShapeLike::translate(cpy, offset_);
        tr_cache_ = cpy; tr_cache_valid_ = true;

        return cpy;
    }

    inline RawShape& rawShape() BP2D_NOEXCEPT { return sh_; }

    inline const RawShape& rawShape() const BP2D_NOEXCEPT { return sh_; }

    inline void reset() BP2D_NOEXCEPT {
        has_offset_ = false; has_rotation_ = false;
    }

    //Static methods:

    inline static bool intersects(const _Item& sh1, const _Item& sh2) {
        return ShapeLike::intersects(sh1.transformedShape(),
                                     sh2.transformedShape());
    }

    inline static bool touches(const _Item& sh1, const _Item& sh2) {
        return ShapeLike::touches(sh1.transformedShape(),
                                  sh2.transformedShape());
    }
};

template<class RawShape>
class _Rectangle: public _Item<RawShape> {
    RawShape sh_;
    using _Item<RawShape>::vertex;
    using TO = Orientation;
public:

    using Unit =  TCoord<RawShape>;

    template<TO o = OrientationType<RawShape>::Value>
    inline _Rectangle(Unit width, Unit height,
                      // disable this ctor if o != CLOCKWISE
                      std::enable_if_t< o == TO::CLOCKWISE, int> = 0 ):
        _Item<RawShape>( ShapeLike::create<RawShape>( {
                                                        {0, 0},
                                                        {0, height},
                                                        {width, height},
                                                        {width, 0},
                                                        {0, 0}
                                                      } ))
    {
    }

    template<TO o = OrientationType<RawShape>::Value>
    inline _Rectangle(Unit width, Unit height,
                      // disable this ctor if o != COUNTER_CLOCKWISE
                      std::enable_if_t< o == TO::COUNTER_CLOCKWISE, int> = 0 ):
        _Item<RawShape>( ShapeLike::create<RawShape>( {
                                                        {0, 0},
                                                        {width, 0},
                                                        {width, height},
                                                        {0, height},
                                                        {0, 0}
                                                      } ))
    {
    }

    inline Unit width() const BP2D_NOEXCEPT {
        return getX(vertex(2));
    }

    inline Unit height() const BP2D_NOEXCEPT {
        return getY(vertex(2));
    }
};

template<class PlacementStrategy>
class PlacementStrategyLike {
    PlacementStrategy impl_;
public:
    using Item = typename PlacementStrategy::Item;
    using Unit = typename PlacementStrategy::Unit;
    using Config = typename PlacementStrategy::Config;
    using BinType = typename PlacementStrategy::BinType;
    using ItemGroup = typename PlacementStrategy::ItemGroup;

    PlacementStrategyLike(const BinType& bin, const Config& config = Config()):
        impl_(bin)
    {
        configure(config);
    }

    inline void configure(const Config& config) { impl_.configure(config); }

    inline bool pack(Item& item) { return impl_.pack(item); }

    inline const BinType& bin() const { return impl_.bin(); }

    inline void bin(const BinType& bin) { impl_.bin(bin); }

    inline ItemGroup getItems() { return impl_.getItems(); }

    inline double waste() const { return impl_.waste(); }

    inline void clearItems() { impl_.clearItems(); }

};

// SelectionStrategy needs to have default constructor
template<class SelectionStrategy>
class SelectionStrategyLike {
    SelectionStrategy impl_;
public:

    using Config = typename SelectionStrategy::Config;
    using ItemGroup = typename SelectionStrategy::ItemGroup;

    inline void configure(const Config& config) {
        impl_.configure(config);
    }

    template<class TPlacer, class TIterator,
             class TBin = typename PlacementStrategyLike<TPlacer>::BinType,
             class PConfig = typename PlacementStrategyLike<TPlacer>::Config>
    inline void packItems(
            TIterator first,
            TIterator last,
            TBin&& bin,
            PConfig&& config = PConfig() )
    {
        impl_.packItems<TPlacer>(first, last,
                                 std::forward<TBin>(bin),
                                 std::forward<PConfig>(config));
    }

    inline size_t binCount() const { return impl_.binCount(); }

    inline ItemGroup itemsForBin(size_t binIndex) {
        return impl_.itemsForBin(binIndex);
    }
};

template<class RawShape, class PlacementStrategy, class SelectionStrategy >
class _Arranger {
    using TSel = SelectionStrategyLike<SelectionStrategy>;
    TSel selector_;

public:
    using TPlacer = PlacementStrategyLike<PlacementStrategy>;
    using Item = _Item<RawShape>;
    using BinType = typename TPlacer::BinType;
    using PlacementConfig = typename TPlacer::Config;
    using SelectionConfig = typename TSel::Config;

    using PackGroup = std::vector<typename TSel::ItemGroup>;

private:
    BinType bin_;
    PlacementConfig pconfig_;

public:

    template<class TBinType = BinType,
             class PConf = PlacementConfig,
             class SConf = SelectionConfig>
    _Arranger(TBinType&& bin,
              PConf&& pconfig = PConf(),
              SConf&& sconfig = SConf()):
        bin_(std::forward<TBinType>(bin)),
        pconfig_(std::forward<PlacementConfig>(pconfig))
    {
        selector_.configure(std::forward<SelectionConfig>(sconfig));
    }

    template<class TIterator>
    inline PackGroup arrange(TIterator from, TIterator to) {

        selector_.packItems<TPlacer>(from, to, bin_, pconfig_);

        PackGroup ret;

        for(size_t i = 0; i < selector_.binCount(); i++) {
            auto items = selector_.itemsForBin(i);
            ret.push_back(items);
        }

        return ret;
    }

    template<class TIterator>
    inline PackGroup operator() (TIterator from, TIterator to) {
        return arrange(from, to);
    }
};

}

#endif // BINPACK2D_HPP
