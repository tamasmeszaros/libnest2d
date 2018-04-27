#ifndef DJD_HEURISTIC_HPP
#define DJD_HEURISTIC_HPP

#include <list>
#include "../binpack2d.hpp"

namespace binpack2d { namespace strategies {

template<class RawShape>
class _DJDHeuristic {
public:
    using Item = _Item<RawShape>;

    struct Config {
        unsigned max_bins;
    };

private:
    using ItemRef = std::reference_wrapper<Item>;

    using ItemGroup = std::vector<ItemRef>;

    using PackGroup = std::vector<ItemGroup>;

    using Container = typename std::vector<Item>;
    Container store_;
    Config config_;
    PackGroup packed_bins_;

    static const double INITIAL_FILL_PROPORTION;

public:

    inline void configure(const Config& config) {
        config_ = config;
    }

    template<class TPlacer, class TIterator,
             class TBin = typename PlacementStrategyLike<TPlacer>::BinType,
             class PConfig = typename PlacementStrategyLike<TPlacer>::Config>
    void packItems( TIterator first,
                    TIterator last,
                    const TBin& bin,
                    PConfig&& pconfig = PConfig() )
    {
        using Placer = PlacementStrategyLike<TPlacer>;

        const double bin_area = ShapeLike::area<RawShape>(bin);
        const double w = bin_area * 0.1;
        const double INITIAL_FILL_AREA = bin_area*INITIAL_FILL_PROPORTION;

        store_.clear();
        store_.reserve(last-first);
        packed_bins_.clear();

        std::copy(first, last, std::back_inserter(store_));

        std::sort(store_.begin(), store_.end(), [](Item& i1, Item& i2) {
            return i1.area() > i2.area();
        });

        std::list<ItemRef> not_packed(store_.begin(), store_.end());

        std::vector<Placer> placers;

        double free_area = 0;
        double filled_area = 0;

        auto addBin = [ &placers, &free_area, &filled_area, &bin, &pconfig]()
        {
            placers.emplace_back(bin);
            placers.back().configure(pconfig);
            free_area = ShapeLike::area<RawShape>(bin);
            filled_area = 0;
        };

        auto tryOneByOne =
                [&not_packed,  &bin_area, &free_area, &filled_area]
                (Placer& placer, double waste)
        {
            double item_area = 0;
            bool ret = false;
            auto it = not_packed.begin();

            while(it != not_packed.end() && !ret &&
                  free_area - (item_area = it->get().area()) <= waste)
            {
                if(item_area <= free_area && placer.pack(*it) ) {
                    free_area -= item_area;
                    filled_area = bin_area - free_area;
                    auto itmp = it++;
                    not_packed.erase(itmp);
                    ret = true;
                } else it++;
            }

            return ret;
        };

        auto tryGroupsOfTwo =
                [&not_packed, &bin_area, &free_area, &filled_area]
                (Placer& placer, double waste)
        {
            return false;
        };


        auto tryGroupsOfThree =
                [&not_packed, &bin_area, &free_area, &filled_area]
                (Placer& placer, double waste)
        {
            return false;
        };

        addBin();

        double waste = 0;
        while(!not_packed.empty()) {

            auto& placer = placers.back();

            {// Fille the bin up to INITIAL_FILL_PROPORTION of its capacity
                auto it = not_packed.begin();

                while(it != not_packed.end() && filled_area < INITIAL_FILL_AREA)
                {
                    if(placer.pack(*it)) {
                        filled_area += it->get().area();
                        free_area = bin_area - filled_area;
                        auto itmp = it++;
                        not_packed.erase(itmp);
                    } else it++;
                }
            }

            // try pieses one by one
            while(tryOneByOne(placer, waste)) waste = 0;

            // try groups of 2 pieses
            while(tryGroupsOfTwo(placer, waste)) waste = 0;

            // try groups of 3 pieses
            while(tryGroupsOfThree(placer, waste)) waste = 0;

            if(waste < free_area) waste += w;
            else if(!not_packed.empty()) addBin();
        }

        std::for_each(placers.begin(), placers.end(),
                      [this](Placer& placer){
            packed_bins_.push_back(placer.getItems());
        });
    }

    inline size_t binCount() const { return packed_bins_.size(); }

    inline ItemGroup itemsForBin(size_t binIndex) {
        assert(binIndex < packed_bins_.size());
        return packed_bins_[binIndex];
    }

};

template<class RawShape>
const double _DJDHeuristic<RawShape>::INITIAL_FILL_PROPORTION = 1.0/3.0;

}
}

#endif // DJD_HEURISTIC_HPP
