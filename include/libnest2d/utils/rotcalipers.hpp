#ifndef ROTCALIPERS_HPP
#define ROTCALIPERS_HPP

#include <libnest2d/libnest2d.hpp>

namespace libnest2d {

template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> myantipodals(const RawShape& poly)
{
    using Iterator = typename TContour<RawShape>::const_iterator;
    using Line = _Segment<TPoint<RawShape>>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    std::vector<Line> lines; lines.reserve(edges);
    
    auto first = sl::cbegin(poly);
//    auto last = std::prev(sl::cend(poly));
    auto last = sl::cend(poly);
    
//    if(getX(*last) != getX(*first) || getY(*last) != getY(*first)) ++last;

    auto inc = [&first, &last](Iterator& it) {
        ++it; if(it == last) it = first;
    };
    
    auto dec = [&first, &last](Iterator& it) {
        if(it == first) it = last;
        else --it;
    };
    
    auto next = [&inc](Iterator it) { auto t = it; inc(t); return t; };
    auto prev = [&dec](Iterator it) { auto t = it; dec(t); return t; };
    
    auto absfmod = [](double a, double m) { return std::abs(std::fmod(a, m)); };
    
    auto full_anticwise = [absfmod, &next](Iterator mi, Iterator ni) {
        double m = Line(*mi, *next(mi)).angleToXaxis();
        double n = Line(*ni, *next(ni)).angleToXaxis();
        return absfmod(2 * Pi + n - m, 2 * Pi);
    };
    
    auto full_cwise = [&full_anticwise](Iterator m, Iterator n) {
        double a = 2 * Pi - full_anticwise(m, n);
        return a;
    };
    
    auto half_cwise = [&next, &absfmod, &full_cwise] (Iterator m, Iterator n) {
        double ret = absfmod(full_cwise(m, n), Pi);
        return ret;
    };
    
    auto half_anticwise = [&next, &absfmod, &full_anticwise] (Iterator m, Iterator n) {
        double ret = absfmod(full_anticwise(m, n), Pi);
        return ret;
    };
    
    using AnglFn = std::function<double(Iterator, Iterator)>;
    AnglFn fullangl = OrientationType<RawShape>::Value != Orientation::CLOCKWISE ?
                    AnglFn(full_cwise) : AnglFn(full_anticwise);

    AnglFn halfangl = OrientationType<RawShape>::Value != Orientation::CLOCKWISE ?
                AnglFn(half_anticwise) : AnglFn(half_cwise);
    
    auto yield = [&lines, &poly](Iterator a, Iterator b) {
        lines.emplace_back(*a, *b);
    };
    
//    auto predict = [&next, &prev] (Iterator u, Iterator l) {
//        auto uy = getY(*next(u)) - getY(*u);
//        auto lx = getX(*l) - getX(*prev(l));
//        auto ux = getX(*next(u)) - getX(*u);
//        auto ly = getY(*l) - getY(*prev(l));
//        return uy * lx >  ux * ly;
//    };
    
    auto predict = [&next](Iterator v, Iterator a, Iterator b) {
        TPoint<RawShape> r = *next(v) - *v;
        TPoint<RawShape> q = *next(a) - *a;
        TPoint<RawShape> p = *next(b) - *b;
        
        return (getX(r)*getX(p) + getY(r)*getY(p)) * std::sqrt(getX(q) * getX(q) + getY(q)*getY(q)) - 
               (getX(r)*getX(q) + getY(r)*getY(q)) * std::sqrt(getX(p) * getX(p) + getY(p)*getY(p));
    };
    
    Iterator i = first;
    Iterator j = next(first);
    while(fullangl(i, j) < Pi) inc(j);

    auto end = first;
    auto current = i;
    
    while(j != end) {
        auto pr = halfangl(current, next(j)) - halfangl(current, next(i));  //predict(current, next(i), next(j));
        
        if(pr < -1e-5) { yield(current, next(i)); }
        else if(pr > 1e-5) { yield(current, next(j)); }
        else/*(pr == 0)*/ {
            yield(i, j);
            yield(next(i), j);
            yield(i, next(j));
            yield(next(i), next(j));
        }
        
        if(pr <= 0) {
            inc(j); current = j;
        } else { 
            inc(i); current = i;
        } 
    }

    return lines;
}

template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> myantipodals2(const RawShape& poly)
{
    using Iterator = typename TContour<RawShape>::const_iterator;
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    using Line = _Segment<TPoint<RawShape>>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    std::vector<Line> lines; lines.reserve(edges);
    
    std::vector<TPoint<RawShape>> U, L;
    
    RawShape sortedpoly = poly;
    
    std::sort(sl::begin(sortedpoly), sl::end(sortedpoly), 
              [](const Point& v1, const Point& v2)
    {
        Coord x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        auto diff = x1 - x2;
        
        if(std::abs(diff) <= std::numeric_limits<Coord>::epsilon())
            return y1 < y2;
    
        return diff < 0;    
    });
    
    auto dir = [](const Point& p, const Point& q, const Point& r) {
        return (getY(q) - getY(p)) * (getX(r) - getX(p)) -
               (getX(q) - getX(p)) * (getY(r) - getY(p));
    };
    
    auto ik = sl::begin(sortedpoly);
    
    while(ik != sl::end(sortedpoly)) {
        
        while(U.size() > 1 && dir(U[U.size() - 2], U.back(), *ik) <= 0) 
            U.pop_back();
        while(L.size() > 1 && dir(L[L.size() - 2], L.back(), *ik) >= 0) 
            L.pop_back();
        
        U.emplace_back(*ik);
        L.emplace_back(*ik);
        
        ++ik;
    }
    
    auto yield = [&lines](const Point& pi, const Point& pj) {
        lines.emplace_back(pi, pj);
    };
    
    size_t i = 0;
    size_t j = L.size() - 1;
    
    while(i < U.size() - 1 || j > 0 ) {
        yield(U[i], L[j]);
        
        if(i == U.size() - 1) 
            --j;
        else if(j == 0) 
            ++i;
        else {
            auto pr = (getY(U[i+1]) - getY(U[i])) * (getX(L[j]) - getX(L[j-1])) - 
                      (getX(U[i+1]) - getX(U[i])) * (getY(L[j]) - getY(L[j-1]));
            
            // This is not part of the original algorithm but it's necessary
            if(std::abs(pr) <= std::numeric_limits<Coord>::epsilon()) 
                yield(U[i+1], L[j]);
            
            if(pr > 0) ++i;
            else --j;
        }
    }
    
    return lines;
}


template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> myantipodals3(const RawShape& poly)
{
    using Iterator = typename TContour<RawShape>::const_iterator;
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    using Line = _Segment<TPoint<RawShape>>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    std::vector<Line> lines; lines.reserve(edges);
    
    std::vector<TPoint<RawShape>> U, L;
    
    std::vector<Iterator> sorted; sorted.reserve(edges);
    
    std::for_each(sl::begin(poly), sl::end(poly), [&sorted](Iterator i){ sorted.emplace_back(i); });
    
    std::sort(sorted.begin(), sorted.end(), 
              [](Iterator& i1, Iterator& i2)
    {
        Point v1 = *i1, v2 = *i2;
        Coord x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        auto diff = x1 - x2;
        
        if(std::abs(diff) <= std::numeric_limits<Coord>::epsilon())
            return y1 < y2;
    
        return diff < 0;    
    });
    
    auto predict = [](const Point &v, const Point &a, const Point &b) {
        return (getX(v)*getX(b) + getY(v)*getY(b)) * std::sqrt(getX(a) * getX(a) + getY(a)*getY(a)) - 
               (getX(v)*getX(a) + getY(v)*getY(a)) * std::sqrt(getX(b) * getX(b) + getY(b)*getY(b));
    };
    
    Iterator i = sorted.front(), j = sorted.back();
    
    
    
    return lines;
}

template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> antipodals(const RawShape& poly)
{
    return myantipodals2(poly);
}

//template <class RawShape> inline
//std::vector<_Segment<TPoint<RawShape>>> antipodals(const RawShape& poly)
//{    
//    using Iterator = typename TContour<RawShape>::const_iterator;
//    using Line = _Segment<TPoint<RawShape>>;

//    size_t edges = sl::contourVertexCount(poly);
//    if(edges <= 3) return {};

//    std::vector<Line> lines; lines.reserve(edges);
    
//    auto first = sl::cbegin(poly);
//    auto last = std::prev(sl::cend(poly));
////    auto last = sl::cend(poly);
    
//    if(getX(*last) != getX(*first) || getY(*last) != getY(*first)) ++last;

//    auto inc = [&first, &last](Iterator& it) {
//        ++it; if(it == last) it = first;
//    };
    
//    auto next = [&inc](Iterator it) { auto t = it; inc(t); return t; };
    
//    auto absfmod = [](double a, double m) { return std::abs(std::fmod(a, m)); };
    
//    auto full_anticwise = [absfmod, &next](Iterator mi, Iterator ni) {
//        double m = Line(*mi, *next(mi)).angleToXaxis();
//        double n = Line(*ni, *next(ni)).angleToXaxis();
//        return absfmod(2 * Pi + n - m, 2 * Pi);
//    };
    
//    auto full_cwise = [&full_anticwise](Iterator m, Iterator n) {
//        double a = 2 * Pi - full_anticwise(m, n);
//        return a;
//    };
    
//    auto half_cwise = [&next, &absfmod, &full_cwise] (Iterator m, Iterator n) {
//        double ret = absfmod(full_cwise(m, n), Pi);
//        return ret;
//    };
    
//    auto half_anticwise = [&next, &absfmod, &full_anticwise] (Iterator m, Iterator n) {
//        double ret = absfmod(full_anticwise(m, n), Pi);
//        return ret;
//    };
    
//    using AnglFn = std::function<double(Iterator, Iterator)>;
//    AnglFn fullangl = OrientationType<RawShape>::Value != Orientation::CLOCKWISE ?
//                AnglFn(full_cwise) : AnglFn(full_anticwise);
    
//    AnglFn halfangl = OrientationType<RawShape>::Value != Orientation::CLOCKWISE ?
//                AnglFn(half_cwise) : AnglFn(half_cwise);
    
//    auto predict = [&next](Iterator v, Iterator a, Iterator b) {
//        TPoint<RawShape> r = *next(v) - *v;
//        TPoint<RawShape> q = *next(a) - *a;
//        TPoint<RawShape> p = *next(b) - *b;
        
//        return (getX(r)*getX(p) + getY(r)*getY(p)) * std::sqrt(getX(q) * getX(q) + getY(q)*getY(q)) - (getX(r)*getX(q) + getY(r)*getY(q)) * std::sqrt(getX(p) * getX(p) + getY(p)*getY(p)) 
//               ;
//    };

//    auto yield = [&lines, &poly](Iterator a, Iterator b) {
//        lines.emplace_back(*a, *b);
//    };
    
//    Iterator i = first;
//    Iterator j = next(i);

//    while(fullangl(i, j) < Pi) inc(j);
    
//    yield(i, j);

//    auto end = first;
//    auto current = i;
    
//    while(j != end) {
//        auto pr = predict(current, next(i), next(j));
                
//        // Now take care of parallel edges
//        if( halfangl(current, next(i)) == halfangl(current, next(j))) 
//        {
//            yield(next(i), j);
//            yield(i, next(j));
//            yield(next(i), next(j));

//            if(current == i) { inc(j);  }
//            else inc(i);
//        }
//        else { 
//            if ( halfangl(current, next(i)) < halfangl(current, next(j)) ) {
//                inc(j); current = j;
//            } else {
//                inc(i); current = i;
//            }
//            yield(i, j);
//        }
//    }

//    return lines;
//}

template <class RawShape> double diameter(const RawShape& poly)
{
    using Line = _Segment<TPoint<RawShape>>;
    
    std::vector<Line> antip = myantipodals(poly);
    
    double l = 0.0;
    for(const Line& line : antip) l = std::max(l, line.length());

    return l;
}

template <class RawShape> 
std::pair<TPoint<RawShape>, double> enclosingCircle(const RawShape& poly)
{
    using Line = _Segment<TPoint<RawShape>>;
    
    std::vector<Line> antip = antipodals(poly);
    
    auto it = antip.begin(), mit = it; double l = 0.0;
    
    while(it != antip.end()) { 
        if(it->length() > l) { l = it->length(); mit = it; }
        ++it;
    }
    
    TPoint<RawShape> d = (mit->second() - mit->first());
    d = {getX(d) / 2, getY(d) / 2};
    
    TPoint<RawShape> center = mit->first() + d;
    
    return std::make_pair(center, mit->length() / 2);
}

}

#endif // ROTCALIPERS_HPP
