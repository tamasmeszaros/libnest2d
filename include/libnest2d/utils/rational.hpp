#ifndef LIBNEST2D_RATIONAL_HPP
#define LIBNEST2D_RATIONAL_HPP

#include <libnest2d/geometry_traits.hpp>

namespace libnest2d {

template<class T> struct NoGCD {
    T operator()(const T&, const T&) { return T(1); }
};

// A very simple representation of an unnormalized rational number.
// The sign of the denominator is still normalized to be always positive.
template <class T, class GCD = NoGCD<T>, class TD = T> class Rational {
    T num; T den;
    
    inline void normsign() { if(den < 0) { den = -den; num = -num; } }
    inline void normalize() { 
        T n = GCD()(num, den); 
        num /= n;
        den /= n; 
    }
public:
    
    using BaseType = T;
    using DoubleType = TD;
    
    inline Rational(): num(T(0)), den(T(1)) {}
    
    inline explicit Rational(const T& n, const T& d = T(1)): num(n), den(d) 
    {
        normsign();    
        normalize();
    }

    inline bool operator>(const Rational& o) const { 
        return TD(o.den) * num > TD(den) * o.num; 
    }
    
    inline bool operator<(const Rational& o) const { 
        return TD(o.den) * num < TD(den) * o.num; 
    }

    inline bool operator==(const Rational& o) const {
        return TD(o.den) * num == TD(den) * o.num;
    }
    
    inline bool operator!=(const Rational& o) const { return !(*this == o); }
    
    inline bool operator<=(const Rational& o) const { 
        return TD(o.den) * num <= TD(den) * o.num;
    }
    
    inline bool operator>=(const Rational& o) const {
        return TD(o.den) * num >= TD(den) * o.num;
    }
    
    inline bool operator< (const T& v) const { return TD(num) <  TD(v) * den; }
    inline bool operator> (const T& v) const { return TD(num) >  TD(v) * den; }
    inline bool operator<=(const T& v) const { return TD(num) <= TD(v) * den; }
    inline bool operator>=(const T& v) const { return TD(num) >= TD(v) * den; }
    
    inline Rational& operator*=(const Rational& o) {
        num *= o.num; den *= o.den; normalize();
        return *this;
    }

    inline Rational& operator/=(const Rational& o) {
        num *= o.den; den *= o.num; normsign(); normalize(); return *this;
    }
    
    inline Rational& operator+=(const Rational& o) {
        den *= o.den; num = o.den * num + o.num * den; normalize(); return *this;
    }
    
    inline Rational& operator-=(const Rational& o) {
        den *= o.den; num = o.den * num - o.num * den; normalize(); return *this;
    }
    
    inline Rational& operator*=(const T& v) { 
        num *= v; normalize(); return *this; 
    }
    
    inline Rational& operator/=(const T& v) { 
        den *= v; normsign(); normalize(); return *this; 
    }
    
    inline Rational& operator+=(const T& v) { 
        num += v * den; normalize(); return *this; 
    }
    
    inline Rational& operator-=(const T& v) { 
        num -= v * den; normalize(); return *this; 
    }
    
    inline Rational operator*(const T& v) const { auto tmp = *this; tmp *= v; return tmp; }
    inline Rational operator/(const T& v) const { auto tmp = *this; tmp /= v; return tmp; }
    inline Rational operator+(const T& v) const { auto tmp = *this; tmp += v; return tmp; }
    inline Rational operator-(const T& v) const { auto tmp = *this; tmp -= v; return tmp; }
    inline Rational operator-() const { auto tmp = *this; tmp.num = -num; return tmp; }
    
    inline T numerator() const { return num; }
    inline T denominator() const { return den; }
};

template<class T, class R> inline T cast(const R& r, RationalTag, ScalarTag) 
{
    return cast<T>(r.numerator()) / cast<T>(r.denominator());
}

template<class T, class R, class TD = T> inline 
Rational<T, TD> cast(const R& r, RationalTag, RationalTag) 
{
    return Rational<T, TD>(static_cast<T>(r.numerator()), 
                           static_cast<T>(r.denominator()));
}

template<class T, class GCD, class TD> struct _NumTag<Rational<T, GCD, TD>> { 
    using Type = RationalTag; 
};

template<class R> inline R abs(const R& r, RationalTag) 
{
    return R(abs(r.numerator()), abs(r.denumerator()));
}

}

#endif // LIBNEST2D_RATIONAL_HPP
