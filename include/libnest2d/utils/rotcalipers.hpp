#ifndef ROTCALIPERS_HPP
#define ROTCALIPERS_HPP

#include <numeric>
#include <libnest2d/libnest2d.hpp>
#include <boost/rational.hpp>

namespace libnest2d {

template <class T> using Rational = boost::rational<T>;

template <class RawShape> RawShape removeCollinearPoints(const RawShape& sh)
{
    using Point = TPoint<RawShape>;
    
    RawShape ret; sl::reserve(ret, sl::contourVertexCount(sh));
    
    Point eprev = *sl::cbegin(sh) - *std::prev(sl::cend(sh));
    
    auto dotperp = [](const Point& a, const Point& b) {
        return getX(a) * getY(b) - getY(a) * getX(b);
    };
    
    auto it  = sl::cbegin(sh);
    auto itx = std::next(it);
    while (it != sl::cend(sh))
    {
        Point enext = *itx - *it;

        TCoord<Point> dp = dotperp(eprev, enext);
        
        if (std::abs(dp) > Epsilon<TCoord<Point>>::Value) 
            sl::addVertex(ret, *it);
        
        eprev = enext;
        if (++itx == sl::cend(sh)) itx = sl::cbegin(sh);
    }
    
    return ret;
}

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

template <class RawShape> 
std::vector<_Segment<TPoint<RawShape>>> myantipodals(const RawShape& sh) 
{
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    using Iterator = typename TContour<RawShape>::const_iterator;
    using Line = _Segment<TPoint<RawShape>>;
    
    size_t vcount = sl::contourVertexCount(sh);
    if(vcount <= 3) return {};
    
    // compare function for vertex extreme search
    auto xcmp = [](const Point& v1, const Point& v2)
    {
        auto diff = getX(v1) - getX(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getY(v1) < getY(v2);
    
        return diff < 0;    
    };
    
    // We shall find the bottom-most left and top-most right vertex as the 
    // starting points for the algorithm.
    auto minX = std::min_element(sl::cbegin(sh), sl::cend(sh), xcmp);
    auto maxX = std::max_element(sl::cbegin(sh), sl::cend(sh), xcmp);
    
    auto i = minX, j = maxX;
    auto first = sl::cbegin(sh);
    auto last = std::prev(sl::cend(sh));
    
    // If the last vertex is equal to the first, it signals a closed polygon and
    // we should ignore it. Then the minimum number of verices should be 4.
    if(getX(*last) == getX(*first) && getY(*last) == getY(*first)) {
        if(vcount <= 4) return {};
        --last;
    }
    
    // Iterator rotating increment
    auto inc = [&first, &last](Iterator& it) {
       if(it == last) it = first; else ++it;
    };
    
    // Iterator rotating decrement
    auto dec = [&first, &last](Iterator& it) { 
        if(it == first) it = last; else --it;
    };
    
    // rotating move operators on the vertex iterators
    auto next = [&inc](Iterator it) { auto t = it; inc(t); return t; };
    auto prev = [&dec](Iterator it) { auto t = it; dec(t); return t; };
    
    // output vector
    std::vector<Line> lines;
    
    // yielding means adding an element to the output vector
    auto yield = [&lines](Iterator a, Iterator b) {
        lines.emplace_back(*a, *b);
    };
    
    // dot product
    auto dotp = [](const Point& a, const Point& b) {
        return getX(a) * getX(b) + getY(a) * getY(b);
    };
    
    // squared vector magnitude
    auto magnsq = [](const Point& p) {
        return getX(p) * getX(p) + getY(p) * getY(p);
    };
    
    // A numericly robust angle measuring function. The measured edges are
    // the edges 'a' = (A - prev(A)) to 'b' = (B - prev(B)) and 'a' to 
    // 'c' = (C - prev(C)). Instead of calculating the angle with an arcus 
    // function, we use: |a|^2 * sin(phi)^2 
    // 
    // The phi angle should not and can not be greater than Pi/2 and sin is
    // monotone in that interval. <- THIS IS NOT TRUE
    //
    // The above function can be computed with the
    // expression: dot(a, vp)^2 / |v|^2  where v is either 'b' or 'c' and vp is
    // a vector perpendicular to v [vp = (v.y, -v.x)]
    // Source is: 
    // https://www.geometrictools.com/Documentation/MinimumAreaRectangle.pdf
    auto anglecmp = 
            [&sh, &prev, &dotp, &magnsq](Iterator A, Iterator B, Iterator C) 
    {
        Iterator Ap = prev(A);
        Iterator Bp = prev(B);
        Iterator Cp = prev(C);
        
        Point a = *A - *Ap, b = *B - *Bp, c = *C - *Cp;
        Point pb(getY(b), -getX(b)), pc(getY(c), -getX(c));
        
        double dotapb = dotp(a, pb), dotapc = dotp(a, pc);
        
        double ra = dotapb * dotapb / magnsq(b);
        double rc = dotapc * dotapc / magnsq(c);
        double diff = ra - rc;
        
        if(std::abs(diff) < magnsq(a) * std::numeric_limits<double>::epsilon())
            return 0;
        
        return diff < 0 ? -1 : 1;
    };
    
    // Go around the polygon, do the rotating calipers.
    while(j != minX) {
        yield(i, j);
        
        int pr = anglecmp(i, next(i), next(j));
        
        if(pr == 0) { yield(i, next(j)); yield(next(i), j); inc(j); }
            
        if(pr <= 0) inc(i);
        else inc(j);
    }
    
    return lines;
}

template <class RawShape> inline
std::vector<_Segment<TPoint<RawShape>>> antipodals(const RawShape& poly)
{
//    return antipodals_concave(poly);
//    return antipodals_convex(poly);
    return myantipodals(poly);
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

//template <class RawShape> Radians minAreaBoundingBoxRotation(const RawShape& sh) 
//{
//    using Point = TPoint<RawShape>;
//    using Line = _Segment<TPoint<RawShape>>;
    
//    std::vector<Line> antip = antipodals(sh);
    
//    auto lencmp = [](const Line& l1, const Line& l2)
//    {
//        return l1.length() < l2.length();     
//    };
    
//    auto max_it = std::max_element(antip.begin(), antip.end(), lencmp);
    
//    auto p1 = std::find(sl::cbegin(sh), sl::cend(sh), max_it->first());
//    auto p2 = std::find(sl::cbegin(sh), sl::cend(sh), max_it->second());
    
//    auto p1n = p1 == std::prev(sl::cend(sh)) ? sl::cbegin(sh) : std::next(p1);
//    auto p2n = p2 == std::prev(sl::cend(sh)) ? sl::cbegin(sh) : std::next(p2);
    
//    auto p1v = p1 == sl::cbegin(sh) ? std::prev(sl::cend(sh)) : std::prev(p1);
//    auto p2v = p2 == sl::cbegin(sh) ? std::prev(sl::cend(sh)) : std::prev(p2);
    
//    Point pp1 = *p1, pp1n = *p1n, pp2 = *p2, pp2n = *p2n, pp1v = *p1v, pp2v = *p2v;
    
//    std::array<Line, 4> lines = { Line{pp1, pp1n}, Line{pp2, pp2n}, Line{pp1v, pp1}, Line{pp2v, pp2}};
    
//    auto maxl_it = std::max_element(lines.begin(), lines.end(), lencmp);
    
//    Radians r = maxl_it->angleToXaxis();
//    return 2*Pi - r;
//}


template <class RawShape> Radians minAreaBoundingBoxRotation(const RawShape& sh) 
{
    using Point = TPoint<RawShape>;
    using Coord = TCoord<Point>;
    using Iterator = typename TContour<RawShape>::const_iterator;
//    using Line = _Segment<TPoint<RawShape>>;
    
    auto first = sl::cbegin(sh);
    auto last = std::prev(sl::cend(sh));
    
    auto inc = [&first, &last](Iterator& it) {
       if(it == last) it = first; else ++it;
    };
    auto dec = [&first, &last](Iterator& it) { 
        if(it == first) it = last; else --it;
    };
    
//    auto next = [&inc](Iterator it) { auto t = it; inc(t); return t; };
    auto prev = [&dec](Iterator it) { auto t = it; dec(t); return t; };
    
    auto xcmp = [](const Point& v1, const Point& v2)
    {
        Coord x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        auto diff = x1 - x2;
        
        if(std::abs(diff) <= Epsilon<Coord>::Value)
            return y1 < y2;
    
        return diff < 0;    
    };
    
    auto ycmp = [](const Point& v1, const Point& v2)
    {
        Coord x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        auto diff = y1 - y2;
        
        if(std::abs(diff) <= Epsilon<Coord>::Value)
            return x1 < x2;
    
        return diff < 0;    
    };
    
    auto minX = std::min_element(sl::cbegin(sh), sl::cend(sh), xcmp);
    auto maxX = std::max_element(sl::cbegin(sh), sl::cend(sh), xcmp);
    auto minY = std::min_element(sl::cbegin(sh), sl::cend(sh), ycmp);
    auto maxY = std::max_element(sl::cbegin(sh), sl::cend(sh), ycmp);
    
    auto i = minY, j = maxX, k = maxY, l = minX;
    
    auto dot = [](const Point& a, const Point& b) {
        return getX(a) * getX(b) + getY(a) * getY(b);
    };
    
    auto perp = [](const Point& p) { return Point(getY(p), -getX(p)); };
    
    auto dotperp = [](const Point& a, const Point& b) {
        return getX(a) * getY(b) - getY(a) * getX(b);
    };
    
    // squared vector magnitude
    auto magnsq = [](const Point& p) {
        return getX(p) * getX(p) + getY(p) * getY(p);
    };
    
    auto rectarea = [&dot, &magnsq, &perp](const Point& axis, 
            const Point& vb, const Point& vr, const Point& vt, const Point& vl) 
    {
        return double(dot(axis, vr - vl)) * dot(-perp(axis), vt - vb) / magnsq(axis);
    };
    
    auto update = [&sh, &prev, &dot, &dotperp, &perp, &magnsq, &inc]
            (const Point& w, std::array<Iterator, 4>& vertices) 
    {
        Iterator B = vertices[0], Bp = prev(B);
        Iterator R = vertices[1], Rp = prev(R);
        Iterator T = vertices[2], Tp = prev(T);
        Iterator L = vertices[3], Lp = prev(L);
        
        Point b = *B - *Bp, r = *R - *Rp, t = *T - *Tp, l = *L - *Lp;
        Point pw = perp(w);
        
        Coord dotwpb = dotperp(w, b), dotwpr = dotperp(pw, r);
        Coord dotwpt = dotperp(-w, t), dotwpl = dotperp(-pw, l);
        
        std::array<Rational<Coord>, 4> angles;
        angles[0] = Rational<Coord>(dotwpb * dotwpb, magnsq(b));
        angles[1] = Rational<Coord>(dotwpr * dotwpr, magnsq(r));
        angles[2] = Rational<Coord>(dotwpt * dotwpt, magnsq(t));
        angles[3] = Rational<Coord>(dotwpl * dotwpl, magnsq(l));
        
        using AngleIndex = std::pair<Rational<Coord>, size_t>;
        std::vector<AngleIndex> A; A.reserve(4);
//        std::vector<size_t> M; M.reserve(4);
        
        for (size_t i = 3, j = 0; j < 4; i = j++) {
            if(vertices[i] != vertices[j] && angles[i] > 0) {
                auto iv = std::make_pair(angles[i], i);
                auto it = std::lower_bound(A.begin(), A.end(), iv,
                                           [](const AngleIndex& ai, 
                                              const AngleIndex& aj) 
                { 
                    return ai.first < aj.first; 
                });
                
                A.insert(it, iv);
            }
        }
        
        // I don't know what to do in this case... 
        if(A.empty()) return vertices;
       
//        size_t mmin = 4;
        auto amin = A.front().first;
        auto imin = A.front().second;
        for(auto& a : A)
            if(a.first == amin) { 
                inc(vertices[a.second]);
//                M.emplace_back(a.second);
//                if(a.second < mmin) mmin = a.second;
            }
        
//        for(auto& m : M) inc(vertices[m]);
        std::rotate(vertices.begin(), vertices.begin() + amin.second, vertices.end());
        
        return vertices;
    };
    
    Point w(1, 0);
    Point w_min = w;
    double min_area = (getX(*maxX) - getX(*minX)) * (getY(*maxY) - getY(*minY));
    
    std::array<Iterator, 4> vertices = {i, j, k, l};
    
    // We will examine edge count + 1 bounding boxes. The one additional is 
    // the initial axis aligned bounding box
    size_t c = 0, count = sl::contourVertexCount(sh) + 1;
    std::vector<bool> edgemask(count, false);
    long eidx = -1;
    
    while(c++ < count) 
    {
        // Update the support vertices
        vertices = update(w, vertices);
        
        if(eidx >= 0 && edgemask[size_t(eidx)]) { 
            std::cout << "repeating edge " << eidx << std::endl; 
            break;
        }
                
        // get the unnormalized direction vector
        w = *vertices[0] - *prev(vertices[0]);
        
        // get the area of the rotated rectangle
        double rarea = rectarea(w, *vertices[0], *vertices[1], *vertices[2], *vertices[3]);
        
        // Update min area and the direction of the min bounding box;
        if(rarea <= min_area) { w_min = w; min_area = rarea; }
        
        eidx = vertices[0] - sl::cbegin(sh);
        edgemask[size_t(eidx)] = true;
    }
    
    c = 0;
    for(bool em : edgemask) {
        if(!em) std::cout << "Unvisited edge: " << c << std::endl;
        c++;
    }
    
    double ret = std::atan2(getY(w_min), getX(w_min));
    auto s = std::signbit(ret);
    if(s) ret += Pi_2;
    
    return -ret;
}


}

#endif // ROTCALIPERS_HPP
