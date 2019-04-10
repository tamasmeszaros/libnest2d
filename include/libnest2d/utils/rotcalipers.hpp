#ifndef ROTCALIPERS_HPP
#define ROTCALIPERS_HPP

#include <libnest2d/libnest2d.hpp>

namespace libnest2d {

template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> antipodals(const RawShape& poly)
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
    
    auto next = [&inc](Iterator it) { auto t = it; inc(t); return t; };
    
    auto absfmod = [](double a, double m) { return std::abs(std::fmod(a, m)); };
    
    auto full_anticwise = [absfmod, &next](Iterator mi, Iterator ni) {
        double m = Line(*mi, *next(mi)).angleToXaxis();
        double n = Line(*ni, *next(ni)).angleToXaxis();
        return absfmod(2 * Pi + n - m, 2 * Pi);
    };
    
    auto full_cwise = [&full_anticwise](Iterator m, Iterator n) {
        double a =  2 * Pi - full_anticwise(m, n);
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
    
    Iterator i = first;
    Iterator j = next(i);

    while(fullangl(i, j) < Pi) inc(j);
    
    yield(i, j);

    auto end = sl::cbegin(poly);
    auto current = i;
    size_t cntr = 0;
    

    while(j != end && cntr < sl::contourVertexCount(poly)) {

        cntr++;
        if (halfangl(current, next(i)) <= halfangl(current, next(j))) {
            inc(j); current = j;
        } else {
            inc(i); current = i;
        }

        yield(i, j);

        // Now take care of parallel edges
        if(/*j != end && */halfangl(current, next(i)) == halfangl(current, next(j))) 
        {
            yield(next(i), j);
            yield(i, next(j));
            yield(next(i), next(j));

            if(current == i) inc(j);
            else inc(i);
        }
    }

    return lines;
}

template <class RawShape> double diameter(const RawShape& poly)
{
    using Line = _Segment<TPoint<RawShape>>;
    
    std::vector<Line> antip = antipodals(poly);
    
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
