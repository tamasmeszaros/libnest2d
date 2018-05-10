#include "binpack2d.hpp"

#include <vector>

namespace binpack2d {

Radians::operator Degrees() { return *this * 180/Pi; }

Radians::Radians(const Degrees &degs): Double( degs * Pi/180) {}

inline double Radians::toDegrees() { return operator Degrees(); }

}
