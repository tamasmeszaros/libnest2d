#ifndef LIBNEST2D_BIGINT_HPP
#define LIBNEST2D_BIGINT_HPP

#include <functional>
#include <cmath>
#include <cstdint>

#include <libnest2d/common.hpp>

namespace libnest2d {

template<int N> using Bits = std::integral_constant<int, N>;

template<class Bits, class I = std::intmax_t> class BigInt {
    static const int N = Bits::value / (8 * sizeof(I)) + 
                         Bits::value % (8 * sizeof(I)) / 8;
    
    static const int IBase = sizeof(I) * 8;
    
    std::array<I, N> v_;
    int sign_ = 1;
public:
    
    BigInt() = default;
    
    explicit constexpr inline BigInt(I v) : 
        v_({v < 0 ? -v : v}), sign_(v < 0 ? -1 : 1)
    {
        static_assert(std::is_integral<I>::value, 
                      "Only integral types are allowed for BigInt!");
    }
    
    BigInt& operator+=(const BigInt& /*o*/) { 
        return *this; 
    }
    BigInt& operator*=(const BigInt& /*o*/) { return *this; }
    BigInt& operator-=(const BigInt& /*o*/) { return *this; }
    BigInt& operator/=(const BigInt& /*o*/) { return *this; }
    
    BigInt& operator+=(I /*o*/) { return *this; }
    BigInt& operator*=(I /*o*/) { return *this; }
    BigInt& operator-=(I /*o*/) { return *this; }
    BigInt& operator/=(I /*o*/) { return *this; }
    
    BigInt operator+(const BigInt& /*o*/) const { return *this; }
    BigInt operator*(const BigInt& /*o*/) const { return *this; }
    BigInt operator-(const BigInt& /*o*/) const { return *this; }
    BigInt operator/(const BigInt& /*o*/) const { return *this; }
    
    BigInt operator+(I /*o*/) const { return *this; }
    BigInt operator*(I /*o*/) const { return *this; }
    BigInt operator-(I /*o*/) const { return *this; }
    BigInt operator/(I /*o*/) const { return *this; }
    
    BigInt operator-() const { 
        auto cpy = *this; sign_ > 0 ? cpy.sign_ = -1 : 1; return cpy; 
    }
    
    bool operator< (I) const { return false; }
    bool operator> (I) const { return false; }
    bool operator<=(I) const { return false; }
    bool operator>=(I) const { return false; }
    bool operator==(I) const { return false; }
    bool operator!=(I) const { return false; }
    
    bool operator< (const BigInt& ) const { return false; }
    bool operator> (const BigInt& ) const { return false; }
    bool operator<=(const BigInt& ) const { return false; }
    bool operator>=(const BigInt& ) const { return false; }
    bool operator==(const BigInt& ) const { return false; }
    bool operator!=(const BigInt& ) const { return false; }
    
    long double to_floating() const {
        long double r = 0.0l; int n = 0; 
        for(I a : v_) r += static_cast<long double>(a) * std::pow(2, IBase * n);
        return r;
    }
    
};

template<int N> using BigInt128 = BigInt<Bits<128>>;
template<int N> using BigInt256 = BigInt<Bits<128>>;
template<int N> using BigInt512 = BigInt<Bits<512>>;

template<class Bits, class I> 
struct _NumTag<BigInt<Bits, I>> { using Type = BigIntTag; };

template<class T, class Bi> T cast(const Bi& r, BigIntTag, ScalarTag) 
{
    static_assert(std::is_floating_point<T>::value, 
                  "BigInt should only be casted to floating point type");
    return static_cast<T>(r.to_floating());
}

template<class Bi> inline Bi abs(const Bi& v, BigIntTag) 
{
    return v < Bi(0) ? -v : v;
}

}

#endif // BIGINT_HPP
