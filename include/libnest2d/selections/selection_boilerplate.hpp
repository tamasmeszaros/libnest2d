#ifndef SELECTION_BOILERPLATE_HPP
#define SELECTION_BOILERPLATE_HPP

#include <atomic>
#include <libnest2d/libnest2d.hpp>

namespace libnest2d { namespace selections {

template<class RawShape>
class SelectionBoilerplate {
public:
    using Item = _Item<RawShape>;
    using ItemRef = std::reference_wrapper<Item>;
    using ItemGroup = std::vector<ItemRef>;
    using PackGroup = std::vector<ItemGroup>;

    size_t binCount() const { return packed_bins_.size(); }

    ItemGroup itemsForBin(size_t binIndex) {
        assert(binIndex < packed_bins_.size());
        return packed_bins_[binIndex];
    }

    inline const ItemGroup itemsForBin(size_t binIndex) const {
        assert(binIndex < packed_bins_.size());
        return packed_bins_[binIndex];
    }

    inline void progressIndicator(ProgressFunction fn) { progress_ = fn; }

    inline void stopCondition(StopCondition cond) { stopcond_ = cond; }

protected:

    PackGroup packed_bins_;
    ProgressFunction progress_ = [](unsigned){};
    StopCondition stopcond_ = [](){ return false; };
};

}
}

#endif // SELECTION_BOILERPLATE_HPP
