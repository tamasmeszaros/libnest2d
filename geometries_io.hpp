#ifndef GEOMETRIES_IO_HPP
#define GEOMETRIES_IO_HPP

#include "geometries.hpp"

#include <ostream>

namespace binpack2d {

inline std::ostream& operator<<(std::ostream& stream, Shape& sh) {
    stream << sh.toString() << std::endl;
    return stream;
}

}

#endif // GEOMETRIES_IO_HPP
