#ifndef MEASURE_HPP
#define MEASURE_HPP

#include <libnest2d/libnest2d.hpp>

#ifdef LIBNEST2D_GEOMETRIES_clipper
#define MM_IN_COORDTTYPE 1000000
#else
#define MM_IN_COORDTTYPE 1
#endif

namespace libnest2d {
template<class T = double> inline BP2D_CONSTEXPR 
Coord mm(enable_if_t<std::is_arithmetic<T>::value, T> val = T(1)) BP2D_NOEXCEPT 
{
    return Coord(val * MM_IN_COORDTTYPE); 
}
}

#endif
