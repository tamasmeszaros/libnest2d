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
    using Coord = TCoord<RawShape>;
    using Vertex = TPoint<RawShape>;
    using Box = _Box<Vertex>;

    // The original shape that gets encapsulated.
    RawShape sh_;

    // Transformation data
    Vertex translation_;
    Radians rotation_;
    Coord offset_distance_;

    // Info about wheter the tranformations will have to take place
    // This is needed because if floating point is used, it is hard to say
    // that a zero angle is not a rotation because of testing for equality.
    bool has_rotation_ = false, has_translation_ = false, has_offset_ = false;

    // For caching the calculations as they can get pretty expensive.
    mutable RawShape tr_cache_;
    mutable bool tr_cache_valid_ = false;
    mutable double area_cache_ = 0;
    mutable bool area_cache_valid_ = false;
    mutable RawShape offset_cache_;
    mutable bool offset_cache_valid_ = false;
public:

    using ShapeType = RawShape;
    using Iterator = TVertexConstIterator<RawShape>;

    static BP2D_CONSTEXPR Orientation orientation() {
        return OrientationType<RawShape>::Value;
    }

    explicit inline _Item(const RawShape& sh): sh_(sh) {}

    explicit inline _Item(RawShape&& sh): sh_(std::move(sh)) {}

    inline _Item(const std::initializer_list< Vertex >& il):
        sh_(ShapeLike::create<RawShape>(il)) {}

    inline std::string toString() const
    {
        return ShapeLike::toString(sh_);
    }

    inline Iterator begin() const
    {
        return ShapeLike::cbegin(sh_);
    }

    inline Iterator cbegin() const
    {
        return ShapeLike::cbegin(sh_);
    }

    inline Iterator end() const
    {
        return ShapeLike::cend(sh_);
    }

    inline Iterator cend() const
    {
        return ShapeLike::cend(sh_);
    }

    inline Vertex vertex(unsigned long idx) const
    {
        return ShapeLike::vertex(sh_, idx);
    }

    inline void setVertex(unsigned long idx,
                          const Vertex& v )
    {
        invalidateCache();
        ShapeLike::vertex(sh_, idx) = v;
    }

    inline double area() const {
        double ret ;
        if(area_cache_valid_) ret = area_cache_;
        else {
            ret = ShapeLike::area(offsettedShape());
            area_cache_ = ret;
            area_cache_valid_ = true;
        }
        return ret;
    }

    inline unsigned long vertexCount() const {
        return ShapeLike::contourVertexCount(sh_);
    }

    inline bool isPointInside(const Vertex& p)
    {
        return ShapeLike::isInside(p, sh_);
    }

    inline bool isInside(const _Item& sh) const
    {
        return ShapeLike::isInside(transformedShape(), sh.transformedShape());
    }

    inline void translate(const Vertex& d) BP2D_NOEXCEPT
    {
        translation_ += d; has_translation_ = true;
        tr_cache_valid_ = false;
    }

    inline void rotate(const Radians& rads) BP2D_NOEXCEPT
    {
        rotation_ += rads;
        has_rotation_ = true;
        tr_cache_valid_ = false;
    }

    inline void addOffset(Coord distance) BP2D_NOEXCEPT
    {
        offset_distance_ = distance;
        has_offset_ = true;
        offset_cache_valid_ = false;
    }

    inline void removeOffset() BP2D_NOEXCEPT {
        has_offset_ = false;
        invalidateCache();
    }

    inline Radians rotation() const BP2D_NOEXCEPT
    {
        return rotation_;
    }

    inline TPoint<RawShape> translation() const BP2D_NOEXCEPT
    {
        return translation_;
    }

    inline RawShape transformedShape() const
    {
        if(tr_cache_valid_) return tr_cache_;

        RawShape cpy = offsettedShape();
        if(has_rotation_) ShapeLike::rotate(cpy, rotation_);
        if(has_translation_) ShapeLike::translate(cpy, translation_);
        tr_cache_ = cpy; tr_cache_valid_ = true;

        return cpy;
    }

    inline operator RawShape() const
    {
        return transformedShape();
    }

    inline const RawShape& rawShape() const BP2D_NOEXCEPT
    {
        return sh_;
    }

    inline void resetTransformation() BP2D_NOEXCEPT
    {
        has_translation_ = false; has_rotation_ = false; has_offset_ = false;
    }

    inline Box boundingBox() const {
        return ShapeLike::boundingBox(transformedShape());
    }

    //Static methods:

    inline static bool intersects(const _Item& sh1, const _Item& sh2)
    {
        return ShapeLike::intersects(sh1.transformedShape(),
                                     sh2.transformedShape());
    }

    inline static bool touches(const _Item& sh1, const _Item& sh2)
    {
        return ShapeLike::touches(sh1.transformedShape(),
                                  sh2.transformedShape());
    }

private:

    inline const RawShape& offsettedShape() const {
        if(has_offset_ ) {
            if(offset_cache_valid_) return offset_cache_;
            else {
                offset_cache_ = sh_;
                ShapeLike::offset(offset_cache_, offset_distance_);
                offset_cache_valid_ = true;
                return offset_cache_;
            }
        }
        return sh_;
    }

    inline void invalidateCache() const BP2D_NOEXCEPT
    {
        tr_cache_valid_ = false;
        area_cache_valid_ = false;
        offset_cache_valid_ = false;
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

    using Unit = TCoord<typename Item::ShapeType>;

    // PackGroup is a table where cells are pairs of an ItemRef with its
    // appropriate index in the input
    using IndexedPackGroup = _IndexedPackGroup<typename Item::ShapeType>;
    using PackGroup = _PackGroup<typename Item::ShapeType>;

private:
    BinType bin_;
    PlacementConfig pconfig_;
    TCoord<typename Item::ShapeType> min_obj_distance_;

    using SItem =  typename SelectionStrategy::Item;
    using TPItem = remove_cvref_t<Item>;
    using TSItem = remove_cvref_t<SItem>;

public:

    template<class TBinType = BinType,
             class PConf = PlacementConfig,
             class SConf = SelectionConfig>
    _Arranger(TBinType&& bin,
              Unit min_obj_distance = 0,
              PConf&& pconfig = PConf(),
              SConf&& sconfig = SConf()):
        bin_(std::forward<TBinType>(bin)),
        pconfig_(std::forward<PlacementConfig>(pconfig)),
        min_obj_distance_(min_obj_distance)
    {
        static_assert( std::is_same<TPItem, TSItem>::value,
                       "Incompatible placement and selection strategy!");

        selector_.configure(std::forward<SelectionConfig>(sconfig));
    }

    template<class TIterator>
    inline PackGroup arrange(TIterator from, TIterator to)
    {
        return _arrange(from, to);
    }

    template<class TIterator>
    inline IndexedPackGroup arrangeIndexed(TIterator from, TIterator to)
    {
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
    inline PackGroup _arrange(TIterator from, TIterator to, bool = false)
    {
        __arrange(from, to);

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

        __arrange(items.begin(), items.end());

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
        __arrange(from, to);
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
        __arrange(items.begin(), items.end());
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

    template<class TIter> inline void __arrange(TIter from, TIter to)
    {
        if(min_obj_distance_ > 0) std::for_each(from, to, [this](Item& item) {
            item.addOffset(min_obj_distance_);
        });

        selector_.template packItems<PlacementStrategy>(
                    from, to, bin_, pconfig_);

        if(min_obj_distance_ > 0) std::for_each(from, to, [this](Item& item) {
            item.removeOffset();
        });

    }
};

}

#endif // BINPACK2D_HPP
