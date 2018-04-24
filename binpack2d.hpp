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
public:

    using Unit =  TCoord<RawShape>;

    _Rectangle(Unit width, Unit height):
        _Item<RawShape>( ShapeLike::create<RawShape>( {
                                                        {0, 0},
                                                        {0, height},
                                                        {width, height},
                                                        {width, 0},
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

    PlacementStrategyLike(const BinType& bin, const Config& config = Config()):
        impl_(bin)
    {
        configure(config);
    }

    inline void configure(const Config& config) { impl_.configure(config); }

    inline bool pack(Item& item) { return impl_.pack(item); }

    inline const BinType& bin() const { return impl_.bin(); }

    inline void bin(const BinType& bin) { impl_.bin(bin); }

    // To read final transformation into item and get the exact placement
    // Packing does not neccessarily calculate final coordinates or
    // transformation. Some placement algorithms work with relative positions
    inline Item& place(Item& item) { return impl_.place(item); }

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

    template<class PlacementLike, class TIterator>
    inline void packItems( PlacementLike&& placer,
                           TIterator first,
                           TIterator last)
    {
        impl_.packItems(placer, first, last);
    }

    inline size_t binCount() const { return impl_.binCount(); }

    inline ItemGroup itemsForBin(size_t binIndex) {
        return impl_.itemsForBin(binIndex);
    }
};

template<class RawShape>
class _BottomLeftPlacementStrategy {
public:
    using Item = _Item<RawShape>;
    using Vertex = TPoint<RawShape>;
    using Segment = _Segment<Vertex>;
    using Box = _Box<Vertex>;
    using BinType = Box;
    using Unit = TCoord<Vertex>;

    struct Config {
        Unit min_obj_distance = 0;
    };

private:
    BinType bin_;
    using Container = std::vector< std::reference_wrapper<Item> > ;
    Container items_;
    Config config_;

public:

    enum class Dir {
        LEFT,
        DOWN
    };

    inline void configure(const Config& config) BP2D_NOEXCEPT {
        config_ = config;
    }

    inline _BottomLeftPlacementStrategy(const BinType& bin):
        bin_(bin) {}

    inline const BinType& bin() const BP2D_NOEXCEPT { return bin_; }
    inline void bin(BinType&& b) { bin_ = std::forward<BinType>(b); }

    inline bool pack(Item& item) {
        return pack(item, config_.min_obj_distance);
    }

    bool pack(Item& item, Unit min_obj_distance) {

        // Get initial position for item in the top right corner
        setInitialPosition(item);

        Unit d = availableSpaceDown(item);
        bool can_move = d > min_obj_distance;
        bool can_be_packed = can_move;
        bool left = true;

        while(can_move) {
            if(left) { // write previous down move and go down
                item.translate({0, -d+min_obj_distance});
                d = availableSpaceLeft(item);
                can_move = d > min_obj_distance;
                left = false;
            } else { // write previous left move and go down
                item.translate({-d+min_obj_distance, 0});
                d = availableSpaceDown(item);
                can_move = d > min_obj_distance;
                left = true;
            }
        }

        if(can_be_packed) {
            items_.push_back(item);
//            bool valid = true;

//            for(Item& r1 : items_) {
//                for(Item& r2 : items_) {
//                    if(&r1 != &r2 ) {
//                        valid = !Item::intersects(r1, r2);
//                        valid = !r1.isInside(r2) && !r2.isInside(r1);

//                        if (!valid) {
//                            can_be_packed = false;
//                        }
//                    }
//                }
//            }
        }

        return can_be_packed;
    }

    inline Item& place(Item& item) const BP2D_NOEXCEPT { return item; }

    inline RawShape leftPoly(const Item& item) const {
        return toWallPoly(item, Dir::LEFT);
    }

    inline RawShape downPoly(const Item& item) const {
        return toWallPoly(item, Dir::DOWN);
    }

    inline Unit availableSpaceLeft(const Item& item) {
        return availableSpace(item, Dir::LEFT);
    }

    inline Unit availableSpaceDown(const Item& item) {
        return availableSpace(item, Dir::DOWN);
    }

protected:
    using Coord = TCoord<Vertex>;

    void setInitialPosition(Item& item) {
        auto bb = ShapeLike::boundingBox(item.rawShape());

        Coord dx = getX(bin_.maxCorner()) - getX(bb.maxCorner());
        Coord dy = getY(bin_.maxCorner()) - getY(bb.maxCorner());

        item.translate({dx, dy});
    }

    template<class C = Coord>
    static std::enable_if_t<std::is_floating_point<C>::value, bool>
    isInTheWayOf( const Item& item,
                  const Item& other,
                  const RawShape& scanpoly)
    {
        auto tsh = other.transformedShape();
        return ( ShapeLike::intersects(tsh, scanpoly) ||
                 ShapeLike::isInside(tsh, scanpoly) ) &&
               ( !ShapeLike::intersects(tsh, item.rawShape()) &&
                 !ShapeLike::isInside(tsh, item.rawShape()) );
    }

    template<class C = Coord>
    static std::enable_if_t<std::is_integral<C>::value, bool>
    isInTheWayOf( const Item& item,
                  const Item& other,
                  const RawShape& scanpoly)
    {
        auto tsh = other.transformedShape();

        bool inters_scanpoly = ShapeLike::intersects(tsh, scanpoly) && !ShapeLike::touches(tsh, scanpoly);
        bool inters_item = ShapeLike::intersects(tsh, item.rawShape()) && !ShapeLike::touches(tsh, item.rawShape());

        return ( inters_scanpoly ||
                 ShapeLike::isInside(tsh, scanpoly)) &&
               ( !inters_item &&
                 !ShapeLike::isInside(tsh, item.rawShape())
                 );
    }

    Container itemsInTheWayOf(const Item& item, const Dir dir) {
        // Get the left or down polygon, that has the same area as the shadow
        // of input item reflected to the left or downwards
        auto&& scanpoly = dir == Dir::LEFT? leftPoly(item) :
                                            downPoly(item);

        Container ret;    // packed items 'in the way' of item
        ret.reserve(items_.size());

        // Predicate to find items that are 'in the way' for left (down) move
        auto predicate = [&scanpoly, &item](const Item& it) {
            return isInTheWayOf(item, it, scanpoly);
        };

        // Get the items that are in the way for the left (or down) movement
        std::copy_if(items_.begin(), items_.end(),
                     std::back_inserter(ret), predicate);

        return ret;
    }

    Unit availableSpace(const Item& _item, const Dir dir) {

        Item item (_item.transformedShape());


        std::function<Coord(const Vertex&)> getCoord;
        std::function< std::pair<Coord, bool>(const Vertex&, const Segment&) >
            availableDistance;

        if(dir == Dir::LEFT) {
            getCoord = [](const Vertex& v) { return getX(v); };
            availableDistance = PointLike::horizontalDistance<Vertex>;
        }
        else {
            getCoord = [](const Vertex& v) { return getY(v); };
            availableDistance = PointLike::verticalDistance<Vertex>;
        }

        auto&& items_to_left = itemsInTheWayOf(item, dir);

        // Comparison function for finding min vertex
        auto cmp = [&getCoord](const Vertex& v1, const Vertex& v2) {
            return getCoord(v1) < getCoord(v2);
        };

        // find minimum left or down coordinate of item
        auto leftmost_vertex_it = std::min_element(item.begin(),
                                                   item.end(),
                                                   cmp);

        // Get the initial distance in floating point
        Unit m = getCoord(*leftmost_vertex_it);


        if(!items_to_left.empty()) { // This is crazy, should be optimized...
            for(Item& pleft : items_to_left) {
                // For all segments in items_to_left

                assert(pleft.vertexCount() > 0);

                auto trpleft = pleft.transformedShape();
                auto first = ShapeLike::begin(trpleft);
                auto next = first + 1;
                auto endit = ShapeLike::end(trpleft);

                while(next != endit) {
                    Segment seg(*(first++), *(next++));
                    for(auto& v : item) {   // For all vertices in item

                        auto d = availableDistance(v, seg);

                        if(d.second && d.first < m)  m = d.first;
                    }
                }
            }
        }

        return m;
    }

    /// Implementation of the left (and down) polygon as described by
    /// [López-Camacho et al. 2013]\
    /// (http://www.cs.stir.ac.uk/~goc/papers/EffectiveHueristic2DAOR2013.pdf)
    /// see algorithm 8 for details...
    RawShape toWallPoly(const Item& _item, const Dir dir) const {
        // The variable names reflect the case of left polygon calculation.
        //
        // We will iterate through the item's vertices and search for the top
        // and bottom vertices (or right and left if dir==Dir::DOWN).
        // Save the relevant vertices and their indices into `bottom` and
        // `top` vectors. In case of left polygon construction these will
        // contain the top and bottom polygons which have the same vertical
        // coordinates (in case there is more of them).
        //
        // We get the leftmost (or downmost) vertex from the `bottom` and `top`
        // vectors and construct the final polygon.

        Item item (_item.transformedShape());

        auto getCoord = [dir](const Vertex& v) {
            return dir == Dir::LEFT? getY(v) : getX(v);
        };

        Coord max_y = std::numeric_limits<Coord>::min();
        Coord min_y = std::numeric_limits<Coord>::max();

        using El = std::pair<size_t, std::reference_wrapper<const Vertex>>;

        std::function<bool(const El&, const El&)> cmp;

        if(dir == Dir::LEFT)
            cmp = [](const El& e1, const El& e2) {
                return getX(e1.second.get()) < getX(e2.second.get());
            };
        else
            cmp = [](const El& e1, const El& e2) {
                return getY(e1.second.get()) < getY(e2.second.get());
            };

        std::vector< El > top;
        std::vector< El > bottom;

        size_t idx = 0;
        for(auto& v : item) { // Find the bottom and top vertices and save them
            auto vref = std::cref(v);
            auto vy = getCoord(v);

            if( vy > max_y ) {
                max_y = vy;
                top.clear();
                top.emplace_back(idx, vref);
            }
            else if(vy == max_y) { top.emplace_back(idx, vref); }

            if(vy < min_y) {
                min_y = vy;
                bottom.clear();
                bottom.emplace_back(idx, vref);
            }
            else if(vy == min_y) { bottom.emplace_back(idx, vref); }

            idx++;
        }

        // Get the top and bottom leftmost vertices, or the right and left
        // downmost vertices (if dir == Dir::DOWN)
        auto topleft_it = std::min_element(top.begin(), top.end(), cmp);
        auto bottomleft_it =
                std::min_element(bottom.begin(), bottom.end(), cmp);

        auto& topleft_vertex = topleft_it->second.get();
        auto& bottomleft_vertex = bottomleft_it->second.get();

        // Start and finish positions for the vertices that will be part of the
        // new polygon
        auto start = std::min(topleft_it->first, bottomleft_it->first);
        auto finish = std::max(topleft_it->first, bottomleft_it->first);

        // the return shape
        RawShape rsh;

        // reserve for all vertices plus 2 for the left horizontal wall
        ShapeLike::reserve(rsh, finish-start+2);

        auto addOthers = [&rsh, finish, start, &item](){
            for(size_t i = start+1; i < finish; i++)
                ShapeLike::addVertex(rsh, item.vertex(i));
        };

        auto reverseAddOthers = [&rsh, finish, start, &item](){
            for(size_t i = finish-1; i > start; i--)
                ShapeLike::addVertex(rsh, item.vertex(i));
        };

        // Final polygon construction...

        if( Item::orientation() == Orientation::CLOCKWISE ) {
            // Clockwise polygon construction
            ShapeLike::addVertex(rsh, topleft_vertex);

            if(dir == Dir::LEFT) reverseAddOthers();
            else {
                ShapeLike::addVertex(rsh, getX(topleft_vertex), 0);
                ShapeLike::addVertex(rsh, getX(bottomleft_vertex), 0);
            }

            ShapeLike::addVertex(rsh, bottomleft_vertex);

            if(dir == Dir::LEFT) {
                ShapeLike::addVertex(rsh, 0, getY(bottomleft_vertex));
                ShapeLike::addVertex(rsh, 0, getY(topleft_vertex));
            }
            else reverseAddOthers();

        } else { // Counter clockwise polygon construction
            /*ShapeLike::addVertex(rsh, topleft_vertex);

            ShapeLike::addVertex(rsh, 0, getCoord(topleft_vertex));
            ShapeLike::addVertex(rsh, 0, getCoord(bottomleft_vertex));

            ShapeLike::addVertex(rsh, bottomleft_vertex);

            if(dir == Dir::LEFT) addOthers();
            else reverseAddOthers();*/
            throw UnimplementedException("Counter clockwise toWallPoly()");
        }

        // Close the polygon
        ShapeLike::addVertex(rsh, topleft_vertex);

        return rsh;
    }
};

template<class RawShape>
class _DummySelectionStrategy {

    using Container = typename std::vector<_Item<RawShape>>;

    Container store_;

public:
    using Item = _Item<RawShape>;
    using ItemRef = std::reference_wrapper<Item>;
    using ItemGroup = std::vector<ItemRef>;
    using PackGroup = std::vector<ItemGroup>;
    using Config = int; //dummy

private:
    PackGroup packed_bins_;

public:

    void configure(const Config& /*config*/) { }

    template<class PlacementLike, class TIterator>
    void packItems(PlacementLike&& placer,
                   TIterator first,
                   TIterator last)
    {

        store_.clear();
        store_.reserve(last-first);
        packed_bins_.clear();

        std::copy(first, last, std::back_inserter(store_));

        size_t pos = 0;

        auto sortfunc = [](Item& i1, Item& i2) {
            return i1.area() > i2.area();
        };

        std::sort(store_.begin(), store_.end(), sortfunc);

        packed_bins_.push_back({});

        for(auto& item : store_ ) {
            if(placer.pack(item)) packed_bins_[pos].push_back(item);
            else {
                packed_bins_.push_back({});
                pos++;
            }
        }
    }

    size_t binCount() const { return packed_bins_.size(); }

    ItemGroup itemsForBin(size_t binIndex) {
        assert(binIndex < packed_bins_.size());
        return packed_bins_[binIndex];
    }

};

template<class RawShape,
         class PlacementStrategy = _BottomLeftPlacementStrategy<RawShape>,
         class SelectionStrategy = _DummySelectionStrategy<RawShape>
         >
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

        TPlacer placer(bin_, pconfig_);
        selector_.packItems(placer, from, to);

        PackGroup ret;

        for(size_t i = 0; i < selector_.binCount(); i++) {
            auto items = selector_.itemsForBin(i);
            std::for_each(items.begin(), items.end(),
                          [&placer](Item& it){
                placer.place(it);
            });
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
