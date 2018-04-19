#ifndef BINPACK2D_H
#define BINPACK2D_H

//#include <binpack2d.hpp>

// The type of backend should be set conditionally by the cmake configuriation
// for now we set it statically to clipper backend
#include "clipper_backend/clipper_backend.hpp"

namespace binpack2d {

using Point = PointImpl;
using Box = _Box<PointImpl>;
using Segment = _Segment<PointImpl>;

using Item = _Item<PolygonImpl>;
using Rectangle = _Rectangle<PolygonImpl>;

template<class SelectionStrategy /* default argument does not work... why? */>
using Arranger = _Arranger<PolygonImpl, SelectionStrategy>;

using DummySelectionStrategy = _DummySelectionStrategy<PolygonImpl>;
using DummyPlacementStrategy = _DummyPlacementStrategy<PolygonImpl, Rectangle>;
using DummyArranger = Arranger<DummySelectionStrategy>;

}

#endif // BINPACK2D_H
