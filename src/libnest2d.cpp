#include <libnest2d.h>

namespace libnest2d {

template class Nester<NfpPlacer, FirstFitSelection>;
template class Nester<BottomLeftPlacer, FirstFitSelection>;
template PackGroup Nester<NfpPlacer, FirstFitSelection>::execute(
        std::vector<Item>::iterator, std::vector<Item>::iterator);
template PackGroup Nester<BottomLeftPlacer, FirstFitSelection>::execute(
        std::vector<Item>::iterator, std::vector<Item>::iterator);
}
