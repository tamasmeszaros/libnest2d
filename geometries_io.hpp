#ifndef GEOMETRIES_IO_HPP
#define GEOMETRIES_IO_HPP

#include "binpack2d.hpp"

#include <ostream>

namespace binpack2d {

template<class RawShape>
std::ostream& operator<<(std::ostream& stream, const _Item<RawShape>& sh) {
    stream << ShapeLike::serialize(sh.transformedShape()) << "\n";
    return stream;
}

}

#endif // GEOMETRIES_IO_HPP
