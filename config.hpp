#ifndef BP2D_CONFIG_HPP
#define BP2D_CONFIG_HPP

#define BP2D_NOEXCEPT
#define BP2D_CONSTEXPR

#include <stdexcept>
#include <string>

namespace binpack2d {

class UnimplementedException : public std::exception
{
    std::string info_;
public:

    UnimplementedException(const std::string& info = ""): info_(info) {}

    virtual char const * what() const {
        std::string ret("No usable implementation avalable for function");
        ret += info_.empty()? "!" : std::string(": ") + info_ + "!";
        return ret.c_str();
    }
};

}
#endif // BP2D_CONFIG_HPP
