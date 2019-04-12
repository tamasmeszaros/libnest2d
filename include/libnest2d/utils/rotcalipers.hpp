#ifndef ROTCALIPERS_HPP
#define ROTCALIPERS_HPP

#include <libnest2d/libnest2d.hpp>

namespace libnest2d {

template <class RawShape>
std::vector<_Segment<TPoint<RawShape>>> _antipodals(
        const std::vector<TPoint<RawShape>> &U, 
        const std::vector<TPoint<RawShape>> &L) 
{
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    using Line = _Segment<TPoint<RawShape>>;
    
    std::vector<Line> lines; lines.reserve(2*U.size() + 2*L.size());
    
    std::function<bool(Coord)> predict;
    
    if(is_clockwise<RawShape>()) predict = [](Coord v) { return v < 0; };
    else predict = [](Coord v) { return v > 0; };
    
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
            if(std::abs(pr) <= Epsilon<Coord>::Value) 
                yield(U[i+1], L[j]);
            
            if(predict(pr)) ++i;
            else --j;
        }
    }
    
    return lines;
}

template <class RawShape>
std::vector<_Segment<TPoint<RawShape>>> antipodals_concave(const RawShape& poly)
{
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    std::vector<Point> U, L;
    
    RawShape sortedpoly = poly;
    
    std::sort(sl::begin(sortedpoly), sl::end(sortedpoly), 
              [](const Point& v1, const Point& v2)
    {
        Coord x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        auto diff = x1 - x2;
        
        if(std::abs(diff) <= Epsilon<Coord>::Value)
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
    
    return _antipodals<RawShape>(U, L);
}

template <class RawShape>
std::vector<_Segment<TPoint<RawShape>>> antipodals_convex(const RawShape& poly)
{
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    auto cmp = [](const Point& v1, const Point& v2) {
        Coord x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        auto diff = x1 - x2;
        
        if(std::abs(diff) <= Epsilon<Coord>::Value) return y1 < y2;
    
        return diff < 0;    
    };
    
    auto min_it = std::min_element(sl::cbegin(poly), sl::cend(poly), cmp);
    auto max_it = std::max_element(sl::cbegin(poly), sl::cend(poly), cmp);
    
    std::vector<Point> U, L;
    
    auto it = min_it;
    while(it != max_it) { 
        L.emplace_back(*it++);
        if(it == sl::cend(poly)) it = sl::cbegin(poly); 
    }
    
    it = min_it;
    while(it != max_it) {
        U.emplace_back(*it); 
        if(it == sl::cbegin(poly)) it = std::prev(sl::cend(poly)); else --it; 
    }
    
    L.emplace_back(*max_it);
    U.emplace_back(*max_it);
    
    return _antipodals<RawShape>(U, L);
}

template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> antipodals(const RawShape& poly)
{
//    return antipodals_concave(poly);
    return antipodals_convex(poly);
}

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
