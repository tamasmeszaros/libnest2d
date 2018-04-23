#ifndef BINPACK2D_H
#define BINPACK2D_H

//#include <binpack2d.hpp>

// The type of backend should be set conditionally by the cmake configuriation
// for now we set it statically to clipper backend
#include "clipper_backend/clipper_backend.hpp"

namespace binpack2d {

using Point = PointImpl;
using Coord = TCoord<PointImpl>;
using Box = _Box<PointImpl>;
using Segment = _Segment<PointImpl>;

using Item = _Item<PolygonImpl>;
using Rectangle = _Rectangle<PolygonImpl>;

using DummySelectionStrategy = _DummySelectionStrategy<PolygonImpl>;
using BottomLeftPlacementStrategy = _BottomLeftPlacementStrategy<PolygonImpl>;

using Arranger = _Arranger<PolygonImpl>;

}

#endif // BINPACK2D_H
