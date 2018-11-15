#ifndef FIRSTFIT_HPP
#define FIRSTFIT_HPP

#include "selection_boilerplate.hpp"

namespace libnest2d { namespace selections {

template<class RawShape>
class _FirstFitSelection: public SelectionBoilerplate<RawShape> {
    using Base = SelectionBoilerplate<RawShape>;
public:
    using typename Base::Item;
    using Config = int; //dummy

private:
    using Base::packed_bins_;
    using typename Base::ItemGroup;
    using Container = ItemGroup;//typename std::vector<_Item<RawShape>>;

    Container store_;

public:

    void configure(const Config& /*config*/) { }

    template<class TPlacer, class TIterator,
             class TBin = typename PlacementStrategyLike<TPlacer>::BinType,
             class PConfig = typename PlacementStrategyLike<TPlacer>::Config>
    void packItems(TIterator first,
                   TIterator last,
                   TBin&& bin,
                   PConfig&& pconfig = PConfig())
    {

        using Placer = PlacementStrategyLike<TPlacer>;

        store_.clear();
        store_.reserve(last-first);
        packed_bins_.clear();

        std::vector<Placer> placers;
        placers.reserve(last-first);

        std::copy(first, last, std::back_inserter(store_));

        auto sortfunc = [](Item& i1, Item& i2) {
            return i1.area() > i2.area();
        };

        std::sort(store_.begin(), store_.end(), sortfunc);

        auto total = last-first;
        auto makeProgress = [this, &total](Placer& placer, size_t idx) {
            packed_bins_[idx] = placer.getItems();
            this->progress_(static_cast<unsigned>(--total));
        };

        auto& cancelled = this->stopcond_;

        // Safety test: try to pack each item into an empty bin. If it fails
        // then it should be removed from the list
        { auto it = store_.begin();
            while (it != store_.end() && !cancelled()) {
                Placer p(bin); p.configure(pconfig);
                if(!p.pack(*it)) {
                    it = store_.erase(it);
                } else it++;
            }
        }


        auto it = store_.begin();

        while(it != store_.end() && !cancelled()) {
            bool was_packed = false;
            size_t j = 0;
            while(!was_packed && !cancelled()) {
                for(; j < placers.size() && !was_packed && !cancelled(); j++) {
                    if((was_packed = placers[j].pack(*it, rem(it, store_) )))
                            makeProgress(placers[j], j);
                }

                if(!was_packed) {
                    placers.emplace_back(bin);
                    placers.back().configure(pconfig);
                    packed_bins_.emplace_back();
                    j = placers.size() - 1;
                }
            }
            ++it;
        }
    }

};

}
}

#endif // FIRSTFIT_HPP
