#ifndef BINPACK2D_HPP
#define BINPACK2D_HPP

#include <memory>
#include <vector>
#include <map>
#include <array>
#include <algorithm>
#include <limits>

#include "geometry_traits.hpp"

namespace binpack2d {

template<class RawShape>
class _Item {
    RawShape sh_;
    TPoint<RawShape> offset_;
    Radians rotation_;
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
        return ShapeLike::point(sh_, idx);
    }

    inline double area() const {
        return ShapeLike::area(sh_);
    }

    inline unsigned long vertexCount() const BP2D_NOEXCEPT {
        return cend() - cbegin();
    }

    inline static bool intersects(const _Item& sh1, const _Item& sh2) {
        return ShapeLike::intersects(sh1.sh_, sh2.sh_);
    }

    inline bool isPointInside(const TPoint<RawShape>& p) {
        return ShapeLike::isInside(p, sh_);
    }

    inline void translation(const TPoint<RawShape>& d) BP2D_NOEXCEPT {
        offset_ = d;
    }

    inline void rotate(const Radians& rads) BP2D_NOEXCEPT { rotation_ = rads; }

    RawShape transformedShape() {
        RawShape cpy = sh_;
        ShapeLike::rotate(cpy, rotation_);
        ShapeLike::translate(cpy, offset_);
        return cpy;
    }

    inline RawShape& rawShape() BP2D_NOEXCEPT { return sh_; }

    inline const RawShape& rawShape() const BP2D_NOEXCEPT { return sh_; }
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

template<class RawShape, class TBinShape>
class _DummyPlacementStrategy {
    TBinShape bin_;
public:
    using Item = _Item<RawShape>;

    _DummyPlacementStrategy(const TBinShape& bin): bin_(bin) {}

    bool insertItem(Item& /*item*/) { return true; }

//protected:

    using Vertex = TPoint<RawShape>;
    using Coord = TCoord<Vertex>;

    RawShape leftPoly(const Item& item) const {

        assert(item.vertexCount() > 0);

        Coord max_y = std::numeric_limits<Coord>::min();
        Coord min_y = std::numeric_limits<Coord>::max();

        using El = std::pair<size_t, std::reference_wrapper<const Vertex>>;

        std::vector< El > top;
        std::vector< El > bottom;

        size_t idx = 0;
        for(auto& v : item) {
            auto vref = std::cref(v);
            auto vy = getY(v);

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

        auto cmp = [](const El& e1, const El& e2) {
            return getX(e1.second.get()) < getX(e2.second.get());
        };

        auto topleft_it = std::min_element(top.begin(), top.end(), cmp);
        auto bottomleft_it =
                std::min_element(bottom.begin(), bottom.end(), cmp);

        const Vertex& topleft_vertex = topleft_it->second;
        const Vertex& bottomleft_vertex = bottomleft_it->second;

        auto start = std::min(topleft_it->first, bottomleft_it->first);
        auto finish = std::max(topleft_it->first, bottomleft_it->first);

        RawShape rsh;

        // reserve for all vertices plus 2 for the left horizontal wall
        ShapeLike::reserve(rsh, finish-start+2);

        if(Item::orientation() == Orientation::CLOCKWISE) {
            ShapeLike::addVertex(rsh, topleft_vertex);

            for(size_t i = finish-1; i > start; i--)
                ShapeLike::addVertex(rsh, item.vertex(i));

            ShapeLike::addVertex(rsh, bottomleft_vertex);

            ShapeLike::addVertex(rsh, 0, getY(bottomleft_vertex));
            ShapeLike::addVertex(rsh, 0, getY(topleft_vertex));

        } else {
            ShapeLike::addVertex(rsh, topleft_vertex);

            ShapeLike::addVertex(rsh, 0, getY(topleft_vertex));
            ShapeLike::addVertex(rsh, 0, getY(bottomleft_vertex));

            ShapeLike::addVertex(rsh, bottomleft_vertex);

            for(size_t i = start+1; i < finish; i++)
                ShapeLike::addVertex(rsh, item.vertex(i));
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
                 Config config = Config())
    {

        using BinType = decltype(bin);

        sel_strategy_.addItems(from, to);

        auto placer =
                SelectionStrategy::template PlacementStrategy<BinType>(bin);

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
