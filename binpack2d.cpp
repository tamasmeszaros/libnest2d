#include "binpack2d.hpp"

#include <vector>

namespace binpack2d {

Radians::operator Degrees() { return *this * 180/Pi; }

Radians::Radians(const Degrees &degs): Double( degs * Pi/180) {}

const char *UnimplementedException::what() const BP2D_NOEXCEPT {
    std::string ret("No usable implementation avalable for function");
    ret += info_.empty()? "!" : std::string(": ") + info_ + "!";
    return ret.c_str();
}

}
