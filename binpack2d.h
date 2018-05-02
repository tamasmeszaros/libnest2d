#ifndef BINPACK2D_H
#define BINPACK2D_H

// The type of backend should be set conditionally by the cmake configuriation
// for now we set it statically to clipper backend
#include <binpack2d/clipper_backend/clipper_backend.hpp>

#include <binpack2d/placers/bottomleft.hpp>
#include <binpack2d/selections/firstfit.hpp>
#include <binpack2d/selections/filler.hpp>
#include <binpack2d/selections/djd_heuristic.hpp>

namespace binpack2d {

using Point = PointImpl;
using Coord = TCoord<PointImpl>;
using Box = _Box<PointImpl>;
using Segment = _Segment<PointImpl>;

using Item = _Item<PolygonImpl>;
using Rectangle = _Rectangle<PolygonImpl>;

using PackGroup = _PackGroup<PolygonImpl>;

using FillerSelection = strategies::_FillerSelection<PolygonImpl>;
using FirstFitSelection = strategies::_FirstFitSelection<PolygonImpl>;
using DJDHeuristic  = strategies::_DJDHeuristic<PolygonImpl>;

using BottomLeftPlacer = strategies::_BottomLeftPlacer<PolygonImpl>;

using FirstFitArranger = _Arranger<BottomLeftPlacer, FirstFitSelection>;
using FillArranger = _Arranger<BottomLeftPlacer, FillerSelection>;
using DJDArranger = _Arranger<BottomLeftPlacer, DJDHeuristic>;
using Arranger = FillArranger;

}

#endif // BINPACK2D_H
