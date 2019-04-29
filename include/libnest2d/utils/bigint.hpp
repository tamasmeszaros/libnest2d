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
    
    BigInt& operator=(I v) 
    {
        std::fill(v_.begin(), v_.end(), I(0));
        sign_ = 1;
        
	if (v < 0) sign_ = -1, v = -v;
	for (size_t i; v > 0; ++i, v = v >> IBase) v_[i] = (v % IBase);
        return *this;
    }
    
    explicit inline BigInt(I v) 
    {
        static_assert(std::is_integral<I>::value, 
                      "Only integral types are allowed for BigInt!");
        *this = v;
    }
    
//    template<class Integral> explicit inline BigInt(Integral v) {
//        *this = static_cast<I>(v);
//    }
    
    BigInt& operator+=(const BigInt& /*o*/) { return *this; }
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
    
    BigInt operator-() { 
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

template<class Bits, class I> 
struct _NumTag<BigInt<Bits, I>> { using Type = BigIntTag; };

template<class T, class Bi> T cast(const Bi& r, BigIntTag, ScalarTag) 
{
    static_assert(std::is_floating_point<T>::value, 
                  "BigInt should only be casted to floating point type");
    return static_cast<T>(r.to_floating());
}

}

#endif // BIGINT_HPP
