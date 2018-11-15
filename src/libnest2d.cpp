#include <libnest2d.h>

namespace libnest2d {

template PackGroup nest<NfpPlacer, FirstFitSelection, std::vector<Item>&>(
        std::vector<Item>& cont,
        const Box &bin,
        Coord dist,
        const NfpPlacer::Config& pcfg,
        const FirstFitSelection::Config& scfg);

template PackGroup nest<NfpPlacer, FirstFitSelection, std::vector<Item>&>(
    std::vector<Item>& cont,
    const Box& bin,
    ProgressFunction prg,
    StopCondition scond,
    Coord dist,
    const NfpPlacer::Config& pcfg,
    const FirstFitSelection::Config& scfg
);

template PackGroup nest<NfpPlacer, FirstFitSelection, std::vector<Item>>(
    std::vector<Item>&& cont,
    const Box& bin,
    Coord dist,
    const NfpPlacer::Config& pcfg,
    const FirstFitSelection::Config& scfg
);

template PackGroup nest<NfpPlacer, FirstFitSelection, std::vector<Item>>(
    std::vector<Item>&& cont,
    const Box& bin,
    ProgressFunction prg,
    StopCondition scond,
    Coord dist,
    const NfpPlacer::Config& pcfg,
    const FirstFitSelection::Config& scfg
);

template
PackGroup nest<NfpPlacer, FirstFitSelection, std::vector<Item>::iterator>(
    std::vector<Item>::iterator from,
    std::vector<Item>::iterator to,
    const Box& bin,
    Coord dist,
    const NfpPlacer::Config& pcfg,
    const FirstFitSelection::Config& scfg
);

template
PackGroup nest<NfpPlacer, FirstFitSelection, std::vector<Item>::iterator>(
    std::vector<Item>::iterator from,
    std::vector<Item>::iterator to,
    const Box& bin,
    ProgressFunction prg,
    StopCondition scond,
    Coord dist,
    const NfpPlacer::Config& pcfg,
    const FirstFitSelection::Config& scfg
);

}
