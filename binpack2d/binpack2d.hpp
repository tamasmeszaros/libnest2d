#ifndef BINPACK2D_HPP
#define BINPACK2D_HPP

#include <memory>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <functional>

#include "geometry_traits.hpp"

namespace binpack2d {

/**
 * An item to be placed on a bin.
 */
template<class RawShape>
class _Item {
    RawShape sh_;
    TPoint<RawShape> offset_;
    Radians rotation_;
    bool has_rotation_ = false, has_offset_ = false;

    // For caching the transformation
    mutable RawShape tr_cache_;
    mutable bool tr_cache_valid_ = false;
    mutable double area_cache_ = 0;
    mutable bool area_cache_valid_ = false;
public:

    using ShapeType = RawShape;

    static BP2D_CONSTEXPR Orientation orientation() {
        return OrientationType<RawShape>::Value;
    }

    explicit inline _Item(const RawShape& sh): sh_(sh) {}

    explicit inline _Item(RawShape&& sh): sh_(std::move(sh)) {}

    inline _Item(const std::initializer_list< TPoint<RawShape> >& il):
        sh_(ShapeLike::create<RawShape>(il)) {}

    inline std::string toString() const { return ShapeLike::toString(sh_); }

    inline TVertexConstIterator<RawShape> begin() const {
        return ShapeLike::cbegin(sh_);
    }

    inline TVertexConstIterator<RawShape> cbegin() const {
        return ShapeLike::cbegin(sh_);
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

    inline void setVertex(unsigned long idx,
                          const TPoint<RawShape>& v ) BP2D_NOEXCEPT
    {
        invalidateCache();
        ShapeLike::vertex(sh_, idx) = v;
    }

    inline double area() const {
        double ret ;
        if(area_cache_valid_) ret = area_cache_;
        else {
            ret = ShapeLike::area(sh_);
            area_cache_ = ret;
            area_cache_valid_ = true;
        }
        return ret;
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

    inline Radians rotation() const BP2D_NOEXCEPT { return rotation_; }

    inline TPoint<RawShape> translation() const BP2D_NOEXCEPT {
        return offset_;
    }

    inline RawShape transformedShape() const {
        if(tr_cache_valid_) return tr_cache_;

        RawShape cpy = sh_;
        if(has_rotation_) ShapeLike::rotate(cpy, rotation_);
        if(has_offset_) ShapeLike::translate(cpy, offset_);
        tr_cache_ = cpy; tr_cache_valid_ = true;

        return cpy;
    }

    inline operator RawShape() const { return transformedShape(); }

    inline const RawShape& rawShape() const BP2D_NOEXCEPT { return sh_; }

    inline void resetTransformation() BP2D_NOEXCEPT {
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

private:

    inline void invalidateCache() const BP2D_NOEXCEPT {
        tr_cache_valid_ = false;
        area_cache_valid_ = false;
    }
};

/**
 * Subclass of _Item for regular rectangle items.
 */
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
                      enable_if_t< o == TO::CLOCKWISE, int> = 0 ):
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
                      enable_if_t< o == TO::COUNTER_CLOCKWISE, int> = 0 ):
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

/**
 * A wrapper interface (trait) class for any placement strategy provider.
 */
template<class PlacementStrategy>
class PlacementStrategyLike {
    PlacementStrategy impl_;
public:
    using Item = typename PlacementStrategy::Item;
    using Config = typename PlacementStrategy::Config;
    using BinType = typename PlacementStrategy::BinType;

    using ItemRef = std::reference_wrapper<Item>;
    using ItemGroup = std::vector<ItemRef>;

    PlacementStrategyLike(const BinType& bin, const Config& config = Config()):
        impl_(bin)
    {
        configure(config);
    }

    inline void configure(const Config& config) { impl_.configure(config); }

    inline bool pack(Item& item) { return impl_.pack(item); }

    inline void unpackLast() { impl_.unpackLast(); }

    inline const BinType& bin() const { return impl_.bin(); }

    inline void bin(const BinType& bin) { impl_.bin(bin); }

    inline ItemGroup getItems() { return impl_.getItems(); }

    inline void clearItems() { impl_.clearItems(); }

};


/**
 * A wrapper interface (trait) class for any selections strategy provider.
 */
template<class SelectionStrategy>
class SelectionStrategyLike {
    SelectionStrategy impl_;
public:
    using Item = typename SelectionStrategy::Item;
    using Config = typename SelectionStrategy::Config;

    using ItemRef = std::reference_wrapper<Item>;
    using ItemGroup = std::vector<ItemRef>;

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
        impl_.template packItems<TPlacer>(first, last,
                                 std::forward<TBin>(bin),
                                 std::forward<PConfig>(config));
    }

    inline size_t binCount() const { return impl_.binCount(); }

    inline ItemGroup itemsForBin(size_t binIndex) {
        return impl_.itemsForBin(binIndex);
    }

    inline const ItemGroup itemsForBin(size_t binIndex) const {
        return impl_.itemsForBin(binIndex);
    }
};

template<class RawShape>
using _PackGroup = std::vector<
                        std::vector<
                            std::reference_wrapper<_Item<RawShape>>
                        >
                   >;

template<class RawShape>
using _IndexedPackGroup = std::vector<
                               std::vector<
                                   std::pair<
                                       unsigned,
                                       std::reference_wrapper<_Item<RawShape>>
                                   >
                               >
                          >;

/**
 * The Arranger is the frontend class for the binpack2d library. It takes the
 * input items and outputs the items with the proper transformations to be
 * inside the provided bin.
 */
template<class PlacementStrategy, class SelectionStrategy >
class _Arranger {
    using TSel = SelectionStrategyLike<SelectionStrategy>;
    TSel selector_;

public:
    using Item = typename PlacementStrategy::Item;
    using ItemRef = std::reference_wrapper<Item>;
    using TPlacer = PlacementStrategyLike<PlacementStrategy>;
    using BinType = typename TPlacer::BinType;
    using PlacementConfig = typename TPlacer::Config;
    using SelectionConfig = typename TSel::Config;

    // PackGroup is a table where cells are pairs of an ItemRef with its
    // appropriate index in the input
    using IndexedPackGroup = _IndexedPackGroup<typename Item::ShapeType>;
    using PackGroup = _PackGroup<typename Item::ShapeType>;

private:
    BinType bin_;
    PlacementConfig pconfig_;

    using SItem =  typename SelectionStrategy::Item;
    using TPItem = remove_cvref_t<Item>;
    using TSItem = remove_cvref_t<SItem>;

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
        static_assert( std::is_same<TPItem, TSItem>::value,
                       "Incompatible placement and selection strategy");

        selector_.configure(std::forward<SelectionConfig>(sconfig));
    }

    template<class TIterator>
    inline PackGroup arrange(TIterator from, TIterator to) {
        return _arrange(from, to);
    }

    template<class TIterator>
    inline IndexedPackGroup arrangeIndexed(TIterator from, TIterator to) {
        return _arrangeIndexed(from, to);
    }

    template<class TIterator>
    inline PackGroup operator() (TIterator from, TIterator to)
    {
        return _arrange(from, to);
    }

private:

    template<class TIterator,
             class IT = remove_cvref_t<typename TIterator::value_type>,

             // This funtion will be used only if the iterators are pointing to
             // a type compatible with the binpack2d::_Item template.
             // This way we can use references to input elements as they will
             // have to exist for the lifetime of this call.
             class T = enable_if_t< std::is_convertible<TPItem, IT>::value,
                                         IT>
             >
    inline PackGroup _arrange(TIterator from, TIterator to, bool = false) {

        selector_.template packItems<PlacementStrategy>(
                    from, to, bin_, pconfig_);

        PackGroup ret;

        for(size_t i = 0; i < selector_.binCount(); i++) {
            auto items = selector_.itemsForBin(i);
            ret.push_back(items);
        }

        return ret;
    }

    template<class TIterator,
             class IT = remove_cvref_t<typename TIterator::value_type>,
             class T = enable_if_t<!std::is_convertible<TPItem, IT>::value,
                                        IT>
             >
    inline PackGroup _arrange(TIterator from, TIterator to, int = false)
    {
        static std::vector<TPItem> items;

        items = {from, to};

        selector_.template packItems<PlacementStrategy>(
                    items.begin(), items.end(), bin_, pconfig_);

        PackGroup ret;

        for(size_t i = 0; i < selector_.binCount(); i++) {
            auto items = selector_.itemsForBin(i);
            ret.push_back(items);
        }

        return ret;
    }

    template<class TIterator,
             class IT = remove_cvref_t<typename TIterator::value_type>,

             // This funtion will be used only if the iterators are pointing to
             // a type compatible with the binpack2d::_Item template.
             // This way we can use references to input elements as they will
             // have to exist for the lifetime of this call.
             class T = enable_if_t< std::is_convertible<TPItem, IT>::value,
                                         IT>
             >
    inline IndexedPackGroup _arrangeIndexed(TIterator from,
                                            TIterator to,
                                            bool = false)
    {

        selector_.template packItems<PlacementStrategy>(
                    from, to, bin_, pconfig_);

        return createIndexedPackGroup(from, to, selector_);
    }

    template<class TIterator,
             class IT = remove_cvref_t<typename TIterator::value_type>,
             class T = enable_if_t<!std::is_convertible<TPItem, IT>::value,
                                        IT>
             >
    inline IndexedPackGroup _arrangeIndexed(TIterator from,
                                            TIterator to,
                                            int = false)
    {
        static std::vector<TPItem> items;

        items = {from, to};

        selector_.template packItems<PlacementStrategy>(
                    items.begin(), items.end(), bin_, pconfig_);

        return createIndexedPackGroup(from, to, selector_);
    }

    template<class TIterator>
    static IndexedPackGroup createIndexedPackGroup(TIterator from,
                                                   TIterator to,
                                                   TSel& selector)
    {
        IndexedPackGroup pg;
        pg.reserve(selector.binCount());

        for(size_t i = 0; i < selector.binCount(); i++) {
            auto items = selector.itemsForBin(i);
            pg.push_back({});
            pg[i].reserve(items.size());

            for(Item& itemA : items) {
                auto it = from;
                unsigned idx = 0;
                while(it != to) {
                    Item& itemB = *it;
                    if(&itemB == &itemA) break;
                    it++; idx++;
                }
                pg[i].emplace_back(idx, itemA);
            }
        }

        return pg;
    }
};

}

#endif // BINPACK2D_HPP
