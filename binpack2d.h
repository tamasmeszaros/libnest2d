#ifndef BINPACK2D_H
#define BINPACK2D_H

//#include <binpack2d.hpp>

// The type of backend should be set conditionally by the cmake configuriation
// for now we set it statically to clipper backend
#include "clipper_backend/clipper_backend.hpp"

namespace binpack2d {

using Point = PointImpl;

using Shape = _Shape<PolygonImpl>;
using Rectangle = _Rectangle<PolygonImpl>;
using Item = _Item<PolygonImpl>;

template<class SelectionStrategy /* default argument does not work... why? */>
using Arranger = _Arranger<PolygonImpl, SelectionStrategy>;

using DummySelectionStrategy = _DummySelectionStrategy<PolygonImpl>;
using DummyArranger = Arranger<DummySelectionStrategy>;

}

#endif // BINPACK2D_H
