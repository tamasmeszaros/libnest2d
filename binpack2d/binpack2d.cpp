#include "binpack2d.hpp"

#include <vector>

namespace binpack2d {

Radians::operator Degrees() { return *this * 180/Pi; }

Radians::Radians(const Degrees &degs): Double( degs * Pi/180) {}

inline double Radians::toDegrees() { return operator Degrees(); }

const char *UnimplementedException::what() const BP2D_NOEXCEPT {
    basic_txt_ = "No usable implementation avalable for function";
    basic_txt_ += info_.empty()? "!" : std::string(": ") + info_ + "!";
    return basic_txt_.c_str();
}

}
