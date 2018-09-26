#ifndef LIBNEST2D_H
#define LIBNEST2D_H

// The type of backend should be set conditionally by the cmake configuriation
// for now we set it statically to clipper backend
#ifdef LIBNEST2D_BACKEND_CLIPPER
#include <libnest2d/backends/clipper/geometries.hpp>
#endif

#ifdef LIBNEST2D_OPTIMIZER_NLOPT
// We include the stock optimizers for local and global optimization
#include <libnest2d/optimizers/nlopt/subplex.hpp>     // Local subplex for NfpPlacer
#include <libnest2d/optimizers/nlopt/genetic.hpp>     // Genetic for min. bounding box
#endif

#include <libnest2d/libnest2d.hpp>
#include <libnest2d/placers/bottomleftplacer.hpp>
#include <libnest2d/placers/nfpplacer.hpp>
#include <libnest2d/selections/firstfit.hpp>
#include <libnest2d/selections/filler.hpp>
#include <libnest2d/selections/djd_heuristic.hpp>

namespace libnest2d {

using Point = PointImpl;
using Coord = TCoord<PointImpl>;
using Box = _Box<PointImpl>;
using Segment = _Segment<PointImpl>;
using Circle = _Circle<PointImpl>;

using Item = _Item<PolygonImpl>;
using Rectangle = _Rectangle<PolygonImpl>;

using PackGroup = _PackGroup<PolygonImpl>;
using IndexedPackGroup = _IndexedPackGroup<PolygonImpl>;

using FillerSelection = selections::_FillerSelection<PolygonImpl>;
using FirstFitSelection = selections::_FirstFitSelection<PolygonImpl>;
using DJDHeuristic  = selections::_DJDHeuristic<PolygonImpl>;

using NfpPlacer = placers::_NofitPolyPlacer<PolygonImpl>;
using BottomLeftPlacer = placers::_BottomLeftPlacer<PolygonImpl>;

template<class Iterator,
         class BinType,
         class Placer = placers::_NofitPolyPlacer<PolygonImpl, BinType>,
         class Selector = FirstFitSelection> 
PackGroup nest(Iterator from, Iterator to, const BinType& bin,
               Coord dist = 0,
               const typename Placer::Config& pconf = {},
               const typename Selector::Config& sconf = {})
{
    Nester<Placer, Selector> nester(bin, dist, pconf, sconf);
    return nester.execute(from, to);
}

template<class Container,
         class BinType,
         class Placer = placers::_NofitPolyPlacer<PolygonImpl, BinType>,
         class Selector = FirstFitSelection>
PackGroup nest(Container&& cont, const BinType& bin,
               Coord dist = 0,
               const typename Placer::Config& pconf = {},
               const typename Selector::Config& sconf = {})
{
    return nest(cont.begin(), cont.end(), bin, dist, pconf, sconf);
}

template<class Iterator,
         class BinType,
         class Placer = placers::_NofitPolyPlacer<PolygonImpl, BinType>,
         class Selector = FirstFitSelection>
PackGroup nest(Iterator from, Iterator to, const BinType& bin,
               ProgressFunction prg,
               StopCondition scond = []() { return false; },
               Coord dist = 0,
               const typename Placer::Config& pconf = {},
               const typename Selector::Config& sconf = {})
{
    Nester<Placer, Selector> nester(bin, dist, pconf, sconf);
    if(prg) nester.progressIndicator(prg);
    if(scond) nester.stopCondition(scond);
    return nester.execute(from, to);
}

template<class Container,
         class BinType,
         class Placer = placers::_NofitPolyPlacer<PolygonImpl, BinType>,
         class Selector = FirstFitSelection>
PackGroup nest(Container&& cont, const BinType& bin,
               ProgressFunction prg,
               StopCondition scond = []() { return false; },
               Coord dist = 0,
               const typename Placer::Config& pconf = {},
               const typename Selector::Config& sconf = {})
{
    return nest(cont.begin(), cont.end(), bin, prg, scond, dist, pconf, sconf);
}

}

#endif // LIBNEST2D_H
