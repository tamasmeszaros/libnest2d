#ifndef LIBNEST2D_RATIONAL_HPP
#define LIBNEST2D_RATIONAL_HPP

#include <libnest2d/geometry_traits.hpp>

namespace libnest2d {

// A very simple representation of an unnormalized rational number.
// The sign of the denominator is still normalized to be always positive.
template <class T> class Rational {
    T num, den;
    
    inline void normsign() { if(den < 0) { den = -den; num = -num; } }
    
public:
    
    using BaseType = T;
    
    inline Rational(): num(T(0)), den(T(1)) {}
    
    inline explicit Rational(const T& n, const T& d = T(1)): num(n), den(d) 
    {
        normsign();    
    }

    inline bool operator>(const Rational& o) const { 
        return o.den * num > den * o.num; 
    }
    
    inline bool operator<(const Rational& o) const { 
        return o.den * num < den * o.num; 
    }

    inline bool operator==(const Rational& o) const {
        T eps = Epsilon<T>::Value;
        return abs(o.den * num - den * o.num) <= eps;
    }
    
    inline bool operator!=(const Rational& o) const { return !(*this == o); }
    
    inline bool operator<=(const Rational& o) const { 
        T diff = o.den * num - den * o.num;
        T eps = Epsilon<T>::Value;
        return diff < 0 || abs(diff) <= eps;
    }
    
    inline bool operator>=(const Rational& o) const {
        T diff = o.den * num - den * o.num;
        return diff > 0 || abs(diff) <= Epsilon<T>::Value;
    }
    
    inline bool operator< (const T& v) const { return num <  v * den; }
    inline bool operator> (const T& v) const { return num >  v * den; }
    inline bool operator<=(const T& v) const { return num <= v * den; }
    inline bool operator>=(const T& v) const { return num >= v * den; }
    
    inline Rational& operator*=(const Rational& o) {
        num *= o.num; den *= o.den; return *this;
    }

    inline Rational& operator/=(const Rational& o) {
        num *= o.den; den *= o.num; return *this;
    }
    
    inline Rational& operator+=(const Rational& o) {
        den *= o.den; num = o.den * num + o.num * den; return *this;
    }
    
    inline Rational& operator-=(const Rational& o) {
        den *= o.den; num = o.den * num - o.num * den; return *this;
    }
    
    inline Rational& operator*=(const T& v) { num *= v; return *this; }
    inline Rational& operator/=(const T& v) { den *= v; normsign(); return *this; }
    inline Rational& operator+=(const T& v) { num += v * den; return *this; }
    inline Rational& operator-=(const T& v) { num -= v * den; return *this; }
    
    inline Rational operator*(const T& v) const { auto tmp = *this; tmp *= v; return tmp; }
    inline Rational operator/(const T& v) const { auto tmp = *this; tmp /= v; return tmp; }
    inline Rational operator+(const T& v) const { auto tmp = *this; tmp += v; return tmp; }
    inline Rational operator-(const T& v) const { auto tmp = *this; tmp -= v; return tmp; }
    inline Rational operator-() const { auto tmp = *this; num = -num; return tmp; }
    
    inline T numerator() const { return num; }
    inline T denominator() const { return den; }
};

template<class T, class R> inline T cast(const R& r, RationalTag, ScalarTag) 
{
    return cast<T>(r.numerator()) / cast<T>(r.denominator());
}

template<class T, class R> inline 
Rational<T> cast(const R& r, RationalTag, RationalTag) 
{
    return Rational<T>(static_cast<T>(r.numerator()), 
                       static_cast<T>(r.denominator()));
}

template<class T> struct _NumTag<Rational<T>> { using Type = RationalTag; };

template<class R> inline R abs(const R& r, RationalTag) 
{
    return R(abs(r.numerator()), abs(r.denumerator()));
}

}

#endif // LIBNEST2D_RATIONAL_HPP
