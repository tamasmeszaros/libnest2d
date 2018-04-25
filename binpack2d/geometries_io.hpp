#ifndef GEOMETRIES_IO_HPP
#define GEOMETRIES_IO_HPP

#include "binpack2d.hpp"

#include <ostream>

namespace binpack2d {

template<class RawShape>
std::ostream& operator<<(std::ostream& stream, const _Item<RawShape>& sh) {
    stream << sh.toString() << "\n";
    return stream;
}

}

#endif // GEOMETRIES_IO_HPP
