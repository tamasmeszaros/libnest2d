#ifndef DJD_HEURISTIC_HPP
#define DJD_HEURISTIC_HPP

#include <list>
#include "../binpack2d.hpp"
#include "selection_boilerplate.hpp"

namespace binpack2d { namespace strategies {

template<class RawShape>
class _DJDHeuristic: public SelectionBoilerplate<RawShape> {
    using Base = SelectionBoilerplate<RawShape>;
public:
    using typename Base::Item;
    using typename Base::ItemRef;

    struct Config {
        unsigned max_bins;
    };

private:
    using Base::packed_bins_;
    using ItemGroup = typename Base::ItemGroup;

    using Container = ItemGroup;//typename std::vector<Item>;
    Container store_;
    Config config_;

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
        using ItemList = std::list<ItemRef>;

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

        ItemList not_packed(store_.begin(), store_.end());

        std::vector<Placer> placers;

        double free_area = 0;
        double filled_area = 0;
        double waste = 0;

        // Will use a subroutine to add a new bin
        auto addBin = [&placers, &free_area, &filled_area, &bin, &pconfig]()
        {
            placers.emplace_back(bin);
            placers.back().configure(pconfig);
            free_area = ShapeLike::area<RawShape>(bin);
            filled_area = 0;
        };

        // Types for pairs and triplets
        using TPair = std::tuple<ItemRef, ItemRef>;
        using TTriplet = std::tuple<ItemRef, ItemRef, ItemRef>;


        // Method for checking a pair whether it was a pack failure.
        auto check_pair = [](const std::vector<TPair>& wrong_pairs,
                ItemRef i1, ItemRef i2)
        {
            return std::any_of(wrong_pairs.begin(), wrong_pairs.end(),
                               [&i1, &i2](const TPair& pair)
            {
                Item& pi1 = std::get<0>(pair), pi2 = std::get<1>(pair);
                Item& ri1 = i1, ri2 = i2;
                return (&pi1 == &ri1 && &pi2 == &ri2) ||
                       (&pi1 == &ri2 && &pi2 == &ri1);
            });
        };

        // Method for checking if a triplet was a pack failure
        auto check_triplet = [](
                const std::vector<TTriplet>& wrong_triplets,
                ItemRef i1,
                ItemRef i2,
                ItemRef i3)
        {
            return std::any_of(wrong_triplets.begin(),
                               wrong_triplets.end(),
                               [&i1, &i2, &i3](const TTriplet& tripl)
            {
                Item& pi1 = std::get<0>(tripl);
                Item& pi2 = std::get<1>(tripl);
                Item& pi3 = std::get<2>(tripl);
                Item& ri1 = i1, ri2 = i2, ri3 = i3;
                return  (&pi1 == &ri1 && &pi2 == &ri2 && &pi3 == &ri3) ||
                        (&pi1 == &ri1 && &pi2 == &ri3 && &pi3 == &ri2) ||
                        (&pi1 == &ri2 && &pi2 == &ri1 && &pi3 == &ri3) ||
                        (&pi1 == &ri3 && &pi2 == &ri2 && &pi3 == &ri1);
            });
        };

        auto tryOneByOne = // Subroutine to try adding items one by one.
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
                    ret = true;
                } else
                    it++;
            }

            if(ret) not_packed.erase(it);

            return ret;
        };

//        auto tryGroupsOfTwo = // Try adding groups of two items into the bin.
//                [&not_packed, &bin_area, &free_area, &filled_area, &check_pair]
//                (Placer& placer, double waste)
//        {
//            double item_area = 0, largest_area = 0, smallest_area = 0;
//            double second_largest = 0, second_smallest = 0;

//            const auto endit = not_packed.end();

//            if(not_packed.size() < 2)
//                return false; // No group of two items
//            else {
//                largest_area = not_packed.front().get().area();
//                auto itmp = not_packed.begin(); itmp++;
//                second_largest = itmp->get().area();
//                if( free_area - second_largest - largest_area > waste)
//                    return false; // If even the largest two items do not fill
//                    // the bin to the desired waste than we can end here.

//                smallest_area = not_packed.back().get().area();
//                itmp = endit; std::advance(itmp, -2);
//                second_smallest = itmp->get().area();
//            }

//            bool ret = false;
//            auto it = not_packed.begin();
//            auto it2 = it;

//            std::vector<TPair> wrong_pairs;

//            double largest = second_largest;
//            double smallest= smallest_area;
//            while(it != endit && !ret && free_area -
//                  (item_area = it->get().area()) - largest <= waste )
//            {
//                // if this is the last element, the next smallest is the
//                // previous item
//                auto itmp = it; std::advance(itmp, 1);
//                if(itmp == endit) smallest = second_smallest;

//                if(item_area + smallest <= free_area ) {
//                    auto pr = placer.trypack(*it);

//                    // First would fit
//                    it2 = not_packed.begin();
//                    double item2_area = 0;
//                    while(it2 != endit && pr && !ret && free_area -
//                          (item2_area = it2->get().area()) - item_area <= waste)
//                    {
//                        double area_sum = item_area + item2_area;

//                        if(it == it2 || area_sum > free_area ||
//                                check_pair(wrong_pairs, *it, *it2)) {
//                            it2++; continue;
//                        }

//                        placer.accept(pr);
//                        auto pr2 = placer.trypack(*it2);
//                        if(!pr2) {
//                            placer.unpackLast(); // remove first
//                            pr2 = placer.trypack(*it2);
//                            if(pr2) {
//                                placer.accept(pr2);
//                                auto pr12 = placer.trypack(*it);
//                                if(pr12) {
//                                    placer.accept(pr12);
//                                    ret = true;
//                                } else {
//                                    placer.unpackLast();
//                                }
//                            }
//                        } else {
//                            placer.accept(pr2);
//                            ret = true;
//                        }

//                        if(ret)
//                        { // Second fits as well
//                            free_area -= area_sum;
//                            filled_area = bin_area - free_area;
//                        } else {
//                            wrong_pairs.emplace_back(*it, *it2);
//                            it2++;
//                        }
//                    }
//                    if(!ret) {
//                        // No second item can be placed, so we have to backtrack
//                        it++;
//                    }
//                } else
//                    it++;

//                largest = largest_area;
//            }

//            if(ret) { not_packed.erase(it); not_packed.erase(it2); }

//            return ret;
//        };

        auto tryGroupsOfTwo = // Try adding groups of two items into the bin.
                [&not_packed, &bin_area, &free_area, &filled_area]
                (Placer& placer, double waste)
        {
            double item_area = 0, largest_area = 0, smallest_area = 0;
            double second_largest = 0, second_smallest = 0;

            const auto endit = not_packed.end();

            if(not_packed.size() < 2)
                return false; // No group of two items
            else {
                largest_area = not_packed.front().get().area();
                auto itmp = not_packed.begin(); itmp++;
                second_largest = itmp->get().area();
                if( free_area - second_largest - largest_area > waste)
                    return false; // If even the largest two items do not fill
                    // the bin to the desired waste than we can end here.

                smallest_area = not_packed.back().get().area();
                itmp = endit; std::advance(itmp, -2);
                second_smallest = itmp->get().area();
            }

            bool ret = false;
            auto it = not_packed.begin();
            auto it2 = it;

            double largest = second_largest;
            double smallest= smallest_area;
            while(it != endit && !ret && free_area -
                  (item_area = it->get().area()) - largest <= waste )
            {
                // if this is the last element, the next smallest is the
                // previous item
                auto itmp = it; std::advance(itmp, 1);
                if(itmp == endit) smallest = second_smallest;

                if(item_area + smallest <= free_area && placer.pack(*it)) {
                    // First would fit
                    it2 = not_packed.begin();
                    double item2_area = 0;
                    while(it2 != endit && !ret && free_area -
                          (item2_area = it2->get().area()) - item_area <= waste)
                    {
                        double area_sum = item_area + item2_area;
                        if(it != it2 &&
                                area_sum <= free_area && placer.pack(*it2))
                        { // Second fits as well
                            free_area -= area_sum;
                            filled_area = bin_area - free_area;
                            ret = true;
                        } else it2++;
                    }
                    if(!ret) {
                        // No second item can be placed, so we have to backtrack
                        placer.unpackLast(); it++;
                    }
                } else
                    it++;

                largest = largest_area;
            }

            if(ret) { not_packed.erase(it); not_packed.erase(it2); }

            return ret;
        };


        auto tryGroupsOfThree = // Try adding groups of three items.
                [&not_packed, &bin_area, &free_area, &filled_area,
                &check_pair, &check_triplet]
                (Placer& placer, double waste)
        {

            auto it = not_packed.begin();           // from
            const auto endit = not_packed.end();    // to
            auto it2 = it, it3 = it;

            // Containers for pairs and triplets that were tried before and
            // do not work.
            std::vector<TPair> wrong_pairs;
            std::vector<TTriplet> wrong_triplets;

            // Will be true if a succesfull pack can be made.
            bool ret = false;

            while (it != endit && !ret) { // drill down 1st level

                // We need to determine in each iteration the largest, second
                // largest, smallest and second smallest item in terms of area.

                auto first = not_packed.begin();
                Item& largest = it == first? *std::next(it) : *first;

                auto second = std::next(first);
                Item& second_largest = it == second ? *std::next(it) : *second;

                double area_of_two_largest =
                        largest.area() + second_largest.area();

                // Check if there is enough free area for the item and the two
                // largest item
                if(free_area - it->get().area() - area_of_two_largest > waste)
                    break;

                // Determine the area of the two smallest item.
                auto last = std::prev(endit);
                Item& smallest = it == last? *std::prev(it) : *last;
                auto second_last = std::prev(last);
                Item& second_smallest = it == second_last? *std::prev(it) :
                                                           *second_last;

                // Check if there is enough free area for the item and the two
                // smallest item.
                double area_of_two_smallest =
                        smallest.area() + second_smallest.area();

                // Check for free area and try to pack the 1st item...
                if(it->get().area() + area_of_two_smallest <= free_area &&
                        placer.pack(*it)) {

                    it2 = not_packed.begin();
                    double rem2_area = free_area - largest.area();
                    double a2_sum = it->get().area() + it2->get().area();

                    while(it2 != endit && !ret &&
                          rem2_area - a2_sum <= waste) {  // Drill down level 2

                        if(it == it2 || check_pair(wrong_pairs, *it, *it2)) {
                            it2++;
                            continue;
                        }

                        bool can_pack2 = false;
                        if(!placer.pack(*it2)) { // Cannot pack the second item
                            // Try it in reverse order. Need to unpack the first
                            placer.unpackLast();

                            if(placer.pack(*it2)) {
                                if(placer.pack(*it)) can_pack2 = true;
                                else {
                                    placer.unpackLast();
                                }
                            }
                        } else can_pack2 = true;

                        if(!can_pack2) {
                            wrong_pairs.emplace_back(*it, *it2);
                            it2++;
                            continue;
                        }
                        // Now we have packed a group of 2 items

                        a2_sum = it->get().area() + it2->get().area();

                        // the 'smallest' variable now could be identical with
                        // it2 but we don't bother with that

                        if(a2_sum + smallest.area() <= free_area &&
                                placer.pack(*it2)) {
                            it3 = not_packed.begin();

                            double a3_sum = a2_sum + it3->get().area();

                            while(it3 != endit && !ret &&
                                  free_area - a3_sum <= waste) { // 3rd level

                                if(it3 == it || it3 == it2 ||
                                        check_triplet(wrong_triplets,
                                                      *it, *it2, *it3)) {
                                    it3++;
                                    continue;
                                }

                                bool can_pack3 = placer.pack(*it3);

                                if(!can_pack3) {

                                    // Unpack the first and second candidate
                                    placer.unpackLast();
                                    placer.unpackLast();

                                    std::array<size_t, 3> indices = {1, 2, 3};

                                    std::array<ItemRef, 3>
                                            candidates = {*it, *it2, *it3};

                                    auto tryPack = [&placer, &candidates](
                                            const decltype(indices)& idx)
                                    {
                                        std::array<bool, 3> packed = {false};

                                        for(auto id : idx) packed[id] =
                                                placer.pack(candidates[id]);

                                        bool check =
                                        std::all_of(packed.begin(),
                                                    packed.end(),
                                                    [](bool b) { return b; });

                                        if(!check) for(bool b : packed) if(b)
                                                placer.unpackLast();

                                        return check;
                                    };

                                    bool has_next = true;
                                    while (!can_pack3 && has_next){
                                        can_pack3 = tryPack(indices);
                                        has_next = std::next_permutation(
                                                    indices.begin(),
                                                    indices.end());
                                    };
                                }

                                if(can_pack3) {
                                    // finishit
                                    free_area -= a3_sum;
                                    filled_area = bin_area - free_area;
                                    ret = true;
                                } else {
                                    wrong_triplets.emplace_back(*it, *it2, *it3);
                                    it3++;
                                }

                            } // 3rd while
                        } else { placer.unpackLast(); it2++; } // 2nd packed
                    } // Second while
                } else it++; // 1st item packed
            } // First while

            if(ret) {
                not_packed.erase(it);
                not_packed.erase(it2);
                not_packed.erase(it3);
            }
            return ret;
        };

        addBin();

        // Safety test: try to pack each item into an empty bin. If it fails
        // then it should be removed from the not_packed list
        { auto it = not_packed.begin();
            while (it != not_packed.end()) {
                Placer p(bin);
                if(!p.pack(*it)) {
                    auto itmp = it++;
                    not_packed.erase(itmp);
                } else it++;
            }
        }

        while(!not_packed.empty()) {

            auto& placer = placers.back();

            {// Fill the bin up to INITIAL_FILL_PROPORTION of its capacity
                auto it = not_packed.begin();

                while(it != not_packed.end() &&
                      filled_area < INITIAL_FILL_AREA)
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
//            while(tryGroupsOfThree(placer, waste)) waste = 0;

            if(waste < free_area) waste += w;
            else if(!not_packed.empty()) addBin();
        }

        std::for_each(placers.begin(), placers.end(),
                      [this](Placer& placer){
            packed_bins_.push_back(placer.getItems());
        });
    }
};

/*
 * The initial fill proportion suggested by
 * [López-Camacho et al. 2013]\
 * (http://www.cs.stir.ac.uk/~goc/papers/EffectiveHueristic2DAOR2013.pdf)
 * is one third of the area of bin.
 */
template<class RawShape>
const double _DJDHeuristic<RawShape>::INITIAL_FILL_PROPORTION = 1.0/3.0;

}
}

#endif // DJD_HEURISTIC_HPP
