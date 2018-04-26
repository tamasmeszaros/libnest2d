#ifndef DJD_HEURISTIC_HPP
#define DJD_HEURISTIC_HPP

#include "../binpack2d.hpp"
#include <list>

namespace binpack2d { namespace strategies {

template<class RawShape>
class _DJDHeuristic {

    using Item = _Item<RawShape>;
    using ItemRef = std::reference_wrapper<Item>;

    // SelectionStrategyLike will use this as return type
    using ItemGroup = std::list<ItemRef>;

    using PackGroup = std::list<ItemGroup>;

    struct Config {
        unsigned max_bins;
    };

private:
    using Container = typename std::vector<Item>;
    Container store_;
    Config config_;
    PackGroup packed_bins_;

    static const double INITIAL_FILL = 1/3;

public:

    inline void configure(const Config& config) {
        config_ = config;
    }

    template<class TPlacer, class TIterator,
             class TBin = typename PlacementStrategyLike<TPlacer>::BinType,
             class PConfig = typename PlacementStrategyLike<TPlacer>::Config>
    void packItems( TIterator first,
                    TIterator last,
                    TBin&& bin,
                    PConfig&& config = PConfig() )
    {

        double waste = 0;
        double filled = 0;
        const double w = 0.1;

        store_.clear();
        store_.reserve(last-first);
        packed_bins_.clear();

        std::copy(first, last, std::back_inserter(store_));

        std::sort(store_.begin(), store_.end(), [](Item& i1, Item& i2) {
            return i1.area() > i2.area();
        });

        ItemGroup not_packed(store_.begin(), store_.end());

        std::vector<PlacementStrategyLike<TPlacer>> placers;
        size_t pidx = 0;

        auto addBin = [&placers, &pidx]() {
            placers.emplace_back(bin);
            placers.back().configure(pconfig);
            pidx++;
        };

        addBin();

        while(!not_packed.empty()) {
            auto& placer = placers[pidx];

            for(auto it = store_.begin();
                it != store_.end() && filled < INITIAL_FILL;
                it++)
            {
                not_packed.remove_if([&placer](ItemRef item){
                    return placer.pack(item);
                });
            }
        }

    }

    inline size_t binCount() const { return packed_bins_.size(); }

    inline ItemGroup itemsForBin(size_t binIndex) {
        assert(binIndex < packed_bins_.size());
        return packed_bins_[binIndex];
    }

};

}
}

#endif // DJD_HEURISTIC_HPP
