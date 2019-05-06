#ifndef ROTCALIPERS_HPP
#define ROTCALIPERS_HPP

#include <numeric>

#include <libnest2d/geometry_traits.hpp>
#include <libnest2d/utils/rational.hpp>
#include <libnest2d/utils/bigint.hpp>

#include <cmath>

namespace libnest2d {

template <class RawShape, class Unit = TCompute<RawShape>>
std::vector<_Segment<TPoint<RawShape>>> _antipodals(
        const std::vector<TPoint<RawShape>> &U, 
        const std::vector<TPoint<RawShape>> &L) 
{
    using Point = TPoint<RawShape>;
    using Line = _Segment<TPoint<RawShape>>;
    
    std::vector<Line> lines; lines.reserve(2*U.size() + 2*L.size());
    
    std::function<bool(Unit)> predict;
    
    if(is_clockwise<RawShape>()) predict = [](Unit v) { return v < 0; };
    else predict = [](Unit v) { return v > 0; };
    
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
            auto pr = 
                    (Unit(getY(U[i+1])) - getY(U[i])) * 
                    (Unit(getX(L[j])) - getX(L[j-1])) - 
                    (Unit(getX(U[i+1])) - getX(U[i])) * 
                    (Unit(getY(L[j])) - getY(L[j-1]));
            
            // This is not part of the original algorithm but it's necessary
            if(pr == 0) 
                yield(U[i+1], L[j]);
            
            if(predict(pr)) ++i;
            else --j;
        }
    }
    
    return lines;
}

template <class RawShape, class Unit = TCompute<RawShape>>
std::vector<_Segment<TPoint<RawShape>>> antipodals_concave(const RawShape& poly)
{
    using Point = TPoint<RawShape>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    std::vector<Point> U, L;
    
    RawShape sortedpoly = poly;
    
    std::sort(sl::begin(sortedpoly), sl::end(sortedpoly), 
              [](const Point& v1, const Point& v2)
    {
        Unit x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        return x1 == x2 ? y1 < y2 : x1 < x2;
    });
    
    auto dir = [](const Point& p, const Point& q, const Point& r) {
        return (Unit(getY(q)) - getY(p)) * (Unit(getX(r)) - getX(p)) -
               (Unit(getX(q)) - getX(p)) * (Unit(getY(r)) - getY(p));
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

template <class RawShape, class Unit = TCompute<RawShape>>
std::vector<_Segment<TPoint<RawShape>>> antipodals_convex(const RawShape& poly)
{
    using Point = TPoint<RawShape>;
    
    size_t edges = sl::contourVertexCount(poly);
    if(edges <= 3) return {};
    
    auto cmp = [](const Point& v1, const Point& v2) {
        Unit x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        return x1 == x2 ? y1 < y2 : x1 < x2;   
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

template <class RawShape, class Unit = TCompute<RawShape>> 
std::vector<_Segment<TPoint<RawShape>>> myantipodals(const RawShape& sh) 
{
    using Pt = TPoint<RawShape>;
    using Iterator = typename TContour<RawShape>::const_iterator;
    using Line = _Segment<TPoint<RawShape>>;
    
    size_t vcount = sl::contourVertexCount(sh);
    if(vcount <= 3) return {};
    
    // compare function for vertex extreme search
    auto xcmp = [](const Pt& v1, const Pt& v2)
    {
        Unit x1 = getX(v1), x2 = getX(v2), y1 = getY(v1), y2 = getY(v2);
        return x1 == x2 ? y1 < y2 : x1 < x2;
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
            [&sh, &prev](Iterator A, Iterator B, Iterator C) 
    {
        Iterator Ap = prev(A);
        Iterator Bp = prev(B);
        Iterator Cp = prev(C);
        
        Pt a = *A - *Ap, b = *B - *Bp, c = *C - *Cp;
        Pt pb(getY(b), -getX(b)), pc(getY(c), -getX(c));
        
        Unit dotapb = pl::dot<Pt, Unit>(a, pb), dotapc = pl::dot<Pt, Unit>(a, pc);
        
        Unit ra = dotapb * dotapb / pl::magnsq<Pt, Unit>(b);
        Unit rc = dotapc * dotapc / pl::magnsq<Pt, Unit>(c);
        
        return ra == rc? 0 : ra < rc ? -1 : 1;
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

template <class RawShape, class Unit = TCompute<RawShape>> inline
std::vector<_Segment<TPoint<RawShape>>> antipodals(const RawShape& poly)
{
//    return antipodals_concave(poly);
//    return antipodals_convex(poly);
    return myantipodals<RawShape, Unit>(poly);
}

template <class RawShape, class Unit = TCompute<RawShape>> 
long double diameter(const RawShape& poly)
{
    using Line = _Segment<TPoint<RawShape>>;
    
    std::vector<Line> antip = antipodals(poly);
    
    Unit l = 0.0;
    for(const Line& line : antip) l = std::max(l, line.sqlength());

    return std::sqrt(cast<long double>(l));
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

//template<class C> using TRational = boost::rational<C>;
//template<class C> using TRational = Rational<C>;

template<class Pt, class Unit = TCompute<Pt>> class RotatedBox {
    Pt axis_;
    Unit bottom_ = Unit(0), right_ = Unit(0);
public:
    
    RotatedBox() = default;
    RotatedBox(const Pt& axis, Unit b, Unit r):
        axis_(axis), bottom_(b), right_(r) {}
    
    inline long double area() const { 
        long double asq = pl::magnsq<Pt, long double>(axis_);
        return cast<long double>(bottom_ * right_) / asq;
    }
    
    inline long double width() const { 
        return abs(bottom_) / std::sqrt(pl::magnsq<Pt, long double>(axis_));
    }
    
    inline long double height() const { 
        return abs(right_) / std::sqrt(pl::magnsq<Pt, long double>(axis_));
    }
    
    inline Unit bottom_extent() const { return bottom_; }
    inline Unit right_extent() const { return right_;  }
    inline const Pt& axis() const { return axis_; }
    
    inline Radians angleToX() const {
        double ret = std::atan2(getY(axis_), getX(axis_));
        auto s = std::signbit(ret);
        if(s) ret += Pi_2;
        return -ret;
    }
};

template <class Poly, class Pt = TPoint<Poly>, class Unit = TCompute<Pt>> 
Poly removeCollinearPoints(const Poly& sh, Unit eps = Unit(0))
{
    Poly ret; sl::reserve(ret, sl::contourVertexCount(sh));
    
    Pt eprev = *sl::cbegin(sh) - *std::prev(sl::cend(sh));
    
    auto it  = sl::cbegin(sh);
    auto itx = std::next(it);
    if(itx != sl::cend(sh)) while (it != sl::cend(sh))
    {
        Pt enext = *itx - *it;

        auto dp = pl::dotperp<Pt, Unit>(eprev, enext);
        if(abs(dp) > eps) sl::addVertex(ret, *it);
        
        eprev = enext;
        if (++itx == sl::cend(sh)) itx = sl::cbegin(sh);
        ++it;
    }
    
    return ret;
}

// The area of the bounding rectangle with the axis dir and support vertices
template<class Pt, class Unit = TCompute<Pt>, class R = Rational<Unit>> 
inline R rectarea(const Pt& w, // the axis
                  const Pt& vb, const Pt& vr, 
                  const Pt& vt, const Pt& vl) 
{
    Unit a = pl::dot<Pt, Unit>(w, vr - vl); 
    Unit b = pl::dot<Pt, Unit>(-pl::perp(w), vt - vb);
    R m = R(a) / pl::magnsq<Pt, Unit>(w);
    m = m * b;
    return m;
};

template<class Pt> 
inline long double frectarea(const Pt& w, // the axis
                        const Pt& vb, const Pt& vr, 
                        const Pt& vt, const Pt& vl) 
{
    long double a = pl::dot<Pt, long double>(w, vr - vl); 
    long double b = pl::dot<Pt, long double>(-perp(w), vt - vb);
    long double d = a / pl::magnsq<Pt, long double>(w);
    return d * b;
};

template<class Pt, 
         class Unit = TCompute<Pt>,
         class R = Rational<Unit>,
         class It = typename std::vector<Pt>::const_iterator>
inline R rectarea(const Pt& w, const std::array<It, 4>& rect)
{
    return rectarea<Pt, Unit, R>(w, *rect[0], *rect[1], *rect[2], *rect[3]);
}

template<class Pt, class It = typename std::vector<Pt>::const_iterator> 
inline long double frectarea(const Pt& w, const std::array<It, 4>& rect) 
{
    return frectarea(w, *rect[0], *rect[1], *rect[2], *rect[3]);
};

// This function is only applicable to counter-clockwise oriented convex
// polygons where only two points can be collinear witch each other.
template <class RawShape, 
          class Unit = TCompute<RawShape>, 
          class Ratio = Rational<Unit>> 
RotatedBox<TPoint<RawShape>, Unit> minAreaBoundingBox(const RawShape& sh) 
{
    using Point = TPoint<RawShape>;
    using Iterator = typename TContour<RawShape>::const_iterator;
    using pointlike::dot; using pointlike::magnsq; using pointlike::perp;

    // Get the first and the last vertex iterator
    auto first = sl::cbegin(sh);
    auto last = std::prev(sl::cend(sh));
    
    // Check conditions and return undefined box if input is not sane.
    if(last == first) return {};
    if(getX(*first) == getX(*last) && getY(*first) == getY(*last)) --last;
    if(last - first < 2) return {};
    
    RawShape shcpy; // empty at this point
    {   
        Point p = *first, q = *std::next(first), r = *last;
        
        // Determine orientation from first 3 vertex (should be consistent)
        Unit d = (Unit(getY(q)) - getY(p)) * (Unit(getX(r)) - getX(p)) -
                 (Unit(getX(q)) - getX(p)) * (Unit(getY(r)) - getY(p));
        
        if(d > 0) { 
            // The polygon is clockwise. A flip is needed (for now)
            sl::reserve(shcpy, last - first);
            auto it = last; while(it != first) sl::addVertex(shcpy, *it--);
            sl::addVertex(shcpy, *first);
            first = sl::cbegin(shcpy); last = std::prev(sl::cend(shcpy));
        }
    }
    
    // Cyclic iterator increment
    auto inc = [&first, &last](Iterator& it) {
       if(it == last) it = first; else ++it;
    };
    
    // Cyclic previous iterator
    auto prev = [&first, &last](Iterator it) { 
        return it == first ? last : std::prev(it); 
    };
    
    // Cyclic next iterator
    auto next = [&first, &last](Iterator it) {
        return it == last ? first : std::next(it);    
    };
    
    // Establish initial (axis aligned) rectangle support verices by determining 
    // polygon extremes:
    
    auto it = first;
    Iterator minX = it, maxX = it, minY = it, maxY = it;
    
    do { // Linear walk through the vertices and save the extreme positions
        
        Point v = *it, d = v - *minX;
        if(getX(d) < 0 || (getX(d) == 0 && getY(d) < 0)) minX = it;
        
        d = v - *maxX;
        if(getX(d) > 0 || (getX(d) == 0 && getY(d) > 0)) maxX = it;
        
        d = v - *minY;
        if(getY(d) < 0 || (getY(d) == 0 && getX(d) > 0)) minY = it;
        
        d = v - *maxY;
        if(getY(d) > 0 || (getY(d) == 0 && getX(d) < 0)) maxY = it;
        
    } while(++it != std::next(last));
    
    // Update the vertices defining the bounding rectangle. The rectangle with
    // the smallest rotation is selected and the supporting vertices are 
    // returned in the 'rect' argument.
    auto update = [&next, &inc]
            (const Point& w, std::array<Iterator, 4>& rect) 
    {
        Iterator B = rect[0], Bn = next(B);
        Iterator R = rect[1], Rn = next(R);
        Iterator T = rect[2], Tn = next(T);
        Iterator L = rect[3], Ln = next(L);
        
        Point b = *Bn - *B, r = *Rn - *R, t = *Tn - *T, l = *Ln - *L;
        Point pw = perp(w);
        using Pt = Point;
        
        Unit dotwpb = dot<Pt, Unit>( w, b), dotwpr = dot<Pt, Unit>(-pw, r);
        Unit dotwpt = dot<Pt, Unit>(-w, t), dotwpl = dot<Pt, Unit>( pw, l);
        Unit dw     = magnsq<Pt, Unit>(w);
        
        std::array<Ratio, 4> angles;
        angles[0] = (Ratio(dotwpb) / magnsq<Pt, Unit>(b)) * dotwpb;
        angles[1] = (Ratio(dotwpr) / magnsq<Pt, Unit>(r)) * dotwpr;
        angles[2] = (Ratio(dotwpt) / magnsq<Pt, Unit>(t)) * dotwpt;
        angles[3] = (Ratio(dotwpl) / magnsq<Pt, Unit>(l)) * dotwpl;
        
        using AngleIndex = std::pair<Ratio, size_t>;
        std::vector<AngleIndex> A; A.reserve(4);

        for (size_t i = 3, j = 0; j < 4; i = j++) {
            if(rect[i] != rect[j] && angles[i] < dw) {
                auto iv = std::make_pair(angles[i], i);
                auto it = std::lower_bound(A.begin(), A.end(), iv,
                                           [](const AngleIndex& ai, 
                                              const AngleIndex& aj) 
                { 
                    return ai.first > aj.first; 
                });
                
                A.insert(it, iv);
            }
        }
        
        // The polygon is supposed to be a rectangle.
        if(A.empty()) return false;
       
        auto amin = A.front().first;
        auto imin = A.front().second;
        for(auto& a : A) if(a.first == amin) inc(rect[a.second]);
            
        std::rotate(rect.begin(), rect.begin() + imin, rect.end());
        
        return true;
    };
    
    Point w(1, 0);
    Point w_min = w;
    Ratio minarea((Unit(getX(*maxX)) - getX(*minX)) * 
                  (Unit(getY(*maxY)) - getY(*minY)));
    
    std::array<Iterator, 4> rect = {minY, maxX, maxY, minX};
    std::array<Iterator, 4> minrect = rect;
    
    // We will examine edge count + 1 bounding boxes. The one additional is 
    // the initial axis aligned bounding box
    size_t c = 0, count = last - first;
    std::vector<bool> edgemask(count, false);
    
    while(c++ < count) 
    {   
        // Update the support vertices, if cannot be updated, break the cycle.
        if(! update(w, rect)) break;
        
        size_t eidx = size_t(rect[0] - first);
        
        if(edgemask[eidx]) break;
        edgemask[eidx] = true;
                
        // get the unnormalized direction vector
        w = *rect[0] - *prev(rect[0]);
        
        // get the area of the rotated rectangle
        Ratio rarea = rectarea<Point, Unit, Ratio>(w, rect);
        
        // Update min area and the direction of the min bounding box;
        if(rarea <= minarea) { w_min = w; minarea = rarea; minrect = rect; }
    }
    
    Unit a = dot<Point, Unit>(w_min, *minrect[1] - *minrect[3]);
    Unit b = dot<Point, Unit>(-perp(w_min), *minrect[2] - *minrect[0]);
    RotatedBox<Point, Unit> bb(w_min, a, b);
    
    return bb;
}

template <class RawShape> Radians minAreaBoundingBoxRotation(const RawShape& sh)
{
    return minAreaBoundingBox(sh).angleToX();
}


}

#endif // ROTCALIPERS_HPP
