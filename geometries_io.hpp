#ifndef GEOMETRIES_IO_HPP
#define GEOMETRIES_IO_HPP

#include "geometries.hpp"

#include <ostream>

namespace binpack2d {

template<class RawShape>
std::ostream& operator<<(std::ostream& stream, Shape<RawShape>& sh) {
    stream << sh.toString() << "\n";
    return stream;
}

}

#endif // GEOMETRIES_IO_HPP
