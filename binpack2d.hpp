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

    inline static bool intersects(const _Item& sh1, const _Item& sh2) {
        return ShapeLike::intersects(sh1.sh_, sh2.sh_);
    }

    inline bool isPointInside(const TPoint<RawShape>& p) {
        return ShapeLike::isInside(p, sh_);
    }

    inline void translate(const TPoint<RawShape>& d) BP2D_NOEXCEPT {
        offset_ += d; has_offset_ = true;
    }

    inline void rotate(const Radians& rads) BP2D_NOEXCEPT {
        rotation_ += rads;
        has_rotation_ = true;
    }

    RawShape transformedShape() const {
        RawShape cpy = sh_;
        if(has_rotation_) ShapeLike::rotate(cpy, rotation_);
        if(has_offset_) ShapeLike::translate(cpy, offset_);
        return cpy;
    }

    inline RawShape& rawShape() BP2D_NOEXCEPT { return sh_; }

    inline const RawShape& rawShape() const BP2D_NOEXCEPT { return sh_; }

    inline void reset() BP2D_NOEXCEPT {
        has_offset_ = false; has_rotation_ = false;
    }
};

template<class RawShape>
class _Rectangle: public _Item<RawShape> {
    RawShape sh_;
    using _Item<RawShape>::vertex;
public:

    using Unit =  TCoord<RawShape>;

    _Rectangle(Unit width, Unit height):
        _Item<RawShape>( ShapeLike::create<RawShape>( {{0, 0},
                                             {0, height},
                                             {width, height},
                                             {width, 0},
                                                        {0, 0}} ))
    {
    }

    inline Unit width() const BP2D_NOEXCEPT {
        return getX(vertex(2));
    }

    inline Unit height() const BP2D_NOEXCEPT {
        return getY(vertex(2));
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

    inline _BottomLeftPlacementStrategy(const BinType& bin): bin_(bin) {}

    inline const BinType& bin() const BP2D_NOEXCEPT { return bin_; }

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
                item.translate({0, -d});
                d = availableSpaceLeft(item);
                can_move = d > min_obj_distance;
                left = false;
            } else { // write previous left move and go down
                item.translate({-d, 0});
                d = availableSpaceDown(item);
                can_move = d > min_obj_distance;
                left = true;
            }
        }

        if(can_be_packed) items_.push_back(item);

        return can_be_packed;
    }

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

    Unit availableSpace(const Item& _item, const Dir dir) {

        Item item (_item.transformedShape());

        std::function<Coord(const Vertex&)> getCoord;
        if(dir == Dir::LEFT)
            getCoord = [](const Vertex& v) {
                return getX(v);
            };
        else
            getCoord = [](const Vertex& v) {
                return getY(v);
            };

        // Get the left or down polygon, that has the same area as the shadow
        // of input item reflected to the left or downwards
        auto&& leftp = dir == Dir::LEFT? leftPoly(item) :
                                         downPoly(item);

        Container items_to_left;    // packed items 'in the way' of item
        items_to_left.reserve(items_.size());

        // Predicate to find items that are 'in the way' for left (down) move
        auto predicate = [&leftp, &item](const Item& it) {
            return ( ShapeLike::intersects(it.rawShape(), leftp) ||
                     ShapeLike::isInside(it.rawShape(), leftp) ) &&
                   ( !ShapeLike::intersects(it.rawShape(), item.rawShape()) &&
                     !ShapeLike::isInside(it.rawShape(), item.rawShape()) );
        };

        // Get the items that are in the way for the left (or down) movement
        std::copy_if(items_.begin(), items_.end(),
                     std::back_inserter(items_to_left), predicate);


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
            for(auto& v : item) {   // For all vertices in item
                for(Item& pleft : items_to_left) {
                    // For all segments in items_to_left

                    assert(pleft.vertexCount() > 0);

                    auto first = pleft.begin();
                    auto next = first + 1;
                    while(next != pleft.end()) {
                        auto d = PointLike::horizontalDistance( v,
                                                    Segment(*first, *next) );
                        if(d.first < m) m = d.first;
                        first++; next++;
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

        auto& topleft_vertex = topleft_it->second;
        auto& bottomleft_vertex = bottomleft_it->second;

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
            else addOthers();

            ShapeLike::addVertex(rsh, bottomleft_vertex);

            ShapeLike::addVertex(rsh, 0, getCoord(bottomleft_vertex));
            ShapeLike::addVertex(rsh, 0, getCoord(topleft_vertex));

        } else { // Counter clockwise polygon construction
            ShapeLike::addVertex(rsh, topleft_vertex);

            ShapeLike::addVertex(rsh, 0, getCoord(topleft_vertex));
            ShapeLike::addVertex(rsh, 0, getCoord(bottomleft_vertex));

            ShapeLike::addVertex(rsh, bottomleft_vertex);

            if(dir == Dir::LEFT) addOthers();
            else reverseAddOthers();
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
    unsigned long pos_;

public:

    using Item = _Item<RawShape>;
    using ItemRef = typename std::reference_wrapper<Item>;
    using ItemGroup = typename std::vector<ItemRef>;

    using Config = int; //dummy

    void configure(const Config& /*config*/) { }

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

    template<class PlacementStrategy>
    void packItems(PlacementStrategy& placer) {}

    size_t binCount() const {}

    ItemGroup itemsForBin(size_t binIndex);

};

template<class TSelector>
struct SelectionLike {

    using Item = typename TSelector::Item;
    using Config = typename TSelector::Config;
    using ItemGroup = typename TSelector::ItemGroup;

    inline static void configure(TSelector& selector, Config&& config) {
        selector.configure(std::forward<Config>(config));
    }

    template<class TIterator>
    inline static void addItems(TSelector& selector,
                                TIterator first, TIterator last)
    {
        selector.addItems(first, last);
    }

    template<class PlacementStrategy>
    inline static void packItems(TSelector selector,
                                 PlacementStrategy& placer)
    {
        selector.packItems(placer);
    }

    inline static size_t binCount(const TSelector& selector) {
        return selector.binCount();
    }

    inline static ItemGroup itemsForBin(TSelector& selector, size_t binIndex) {
        return selector.itemsForBin(binIndex);
    }

    inline static TSelector create() {
        return TSelector();
    }
};

template<class TPlacer>
struct PlacementLike {

    using Item = typename TPlacer::Item;
    using Unit = typename TPlacer::Unit;
    using Config = typename TPlacer::Config;
    using BinType = typename TPlacer::BinType;

    inline static void configure(TPlacer& placer, Config&& config) {
        placer.configure(std::forward<Config>(config));
    }

    inline static bool pack(TPlacer& placer, Item& item) {
        placer.pack(item);
    }

    inline static const BinType& bin(const TPlacer& placer) {
        return placer.bin();
    }

    inline static TPlacer create(BinType&& bin) {
        return TPlacer(std::forward<BinType>(bin));
    }
};

template<class RawShape,
         class PlacementStrategy = _BottomLeftPlacementStrategy<RawShape>,
         class SelectionStrategy = _DummySelectionStrategy<RawShape>
         >
class _Arranger {

    PlacementStrategy placer_;
    SelectionStrategy selector_;

public:    
    using TSel = SelectionLike<SelectionStrategy>;
    using TPlacer = PlacementLike<PlacementStrategy>;

    using BinType = typename TPlacer::BinType;
    using PlacementConfig = typename TPlacer::Config;
    using SelectionConfig = typename TSel::Config;

    _Arranger(BinType&& bin,
              PlacementConfig&& pconfig = PlacementConfig(),
              SelectionConfig&& sconfig = SelectionConfig()):
        placer_(TPlacer::create(std::forward<BinType>(bin))),
        selector_(TSel::create())
    {
        TPlacer::configure(placer_, std::forward<PlacementConfig>(pconfig));
        TSel::configure(selector_, std::forward<SelectionConfig>(sconfig));
    }

    template<class TIterator>
    void arrange(TIterator from, TIterator to) {
        TSel::addItems(selector_, from, to);
        TSel::packItems(selector_, placer_);
    }
};

}

#endif // BINPACK2D_HPP
