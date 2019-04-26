#ifndef ROTCALIPERS_HPP
#define ROTCALIPERS_HPP

#include <numeric>

#include <libnest2d/libnest2d.hpp>

#include <boost/rational.hpp>
#include <boost/multiprecision/integer.hpp>

#include <cmath>
namespace std {
boost::multiprecision::int512_t abs(const boost::multiprecision::int512_t& i) { return boost::multiprecision::abs(i); }
}

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

template<class C> using TRational = boost::rational<C>;

// A very simple representation of an unnormalized rational number.
//    class Rational {
//        Coord num, den;
//    public:
//        inline Rational(): num(Coord(0)), den(Coord(1)) {}
//        inline explicit Rational(Coord n, Coord d = Coord(1)): num(n), den(d) {}
    
//        inline bool operator>(const Rational& o) const { 
//            return o.den * num > den * o.num; 
//        }
    
//        inline bool operator==(const Rational& o) const {
//            return std::abs(o.den * num - den * o.num) <= Epsilon<Coord>::Value;
//        }
    
//        inline bool operator!=(const Rational& o) const {
//            return !(*this == o);
//        }
    
//        inline bool operator<(Coord v) const { return num < v * den; }
//        inline bool operator>(Coord v) const { return num > v * den; }
//    };

template<class Pt, class Unit = TCoord<Pt>> class RotatedBox {
    Pt m_axis;
    Unit m_bottom = 0, m_right = 0;
    long double m_area;
public:
    
    RotatedBox() = default;
    RotatedBox(const Pt& axis, Unit b, Unit r, long double ar):
        m_axis(axis), m_bottom(b), m_right(r), m_area(ar) {}
    
    long double area() const { 
//        double asq = double(getX(m_axis)) * getX(m_axis) + 
//                     double(getY(m_axis)) * getY(m_axis);
//        return double(m_bottom) * double(m_right) / asq;
        return m_area;
    }
    
    Radians angleToX() const {
        double ret = std::atan2(getY(m_axis), getX(m_axis));
        auto s = std::signbit(ret);
        if(s) ret += Pi_2;
        return -ret;
    }
};

// create perpendicular vector
template<class Pt> inline Pt perp(const Pt& p) 
{ 
    return Pt(getY(p), -getX(p));
};

template<class Pt, class Unit = TCoord<Pt>> 
inline Unit dotperp(const Pt& a, const Pt& b) 
{ 
    return Unit(getX(a)) * Unit(getY(b)) - Unit(getY(a)) * Unit(getX(b)); 
};

// dot product
template<class Pt, class Unit = TCoord<Pt>> 
inline Unit dot(const Pt& a, const Pt& b) 
{
    Unit x = Unit(getX(a)) * getX(b), y = Unit(getY(a)) * getY(b);
    assert(getX(a) == 0 || x / getX(a) == getX(b));
    assert(getY(a) == 0 || y / getY(a) == getY(b));
    
    Unit ret = x + y;
    
    assert(!((x >= 0 && y >= 0 && ret < 0) || (x <= 0 && y <= 0 && ret > 0)));
    return ret;
};

// squared vector magnitude
template<class Pt, class Unit = TCoord<Pt>> inline Unit magnsq(const Pt& p) 
{
    Unit xx = Unit(getX(p)) * getX(p);
    assert(getX(p) == 0 || xx / getX(p) == getX(p));
    
    Unit yy = Unit(getY(p)) * getY(p);
    assert(getY(p) == 0 || yy / getY(p) == getY(p));
    
    Unit ret = xx + yy;
    assert(!(xx >= 0 && yy >= 0 && ret < 0)||(xx <= 0 && yy <= 0 && ret > 0));
    
    return ret;
};

template <class Poly, class Pt = TPoint<Poly>, class Unit = TCoord<Pt>> 
Poly removeCollinearPoints(const Poly& sh, Unit eps = Unit(0))
{
    Poly ret; sl::reserve(ret, sl::contourVertexCount(sh));
    
    Pt eprev = *sl::cbegin(sh) - *std::prev(sl::cend(sh));
    
    auto it  = sl::cbegin(sh);
    auto itx = std::next(it);
    if(itx != sl::cend(sh)) while (it != sl::cend(sh))
    {
        Pt enext = *itx - *it;

        auto dp = dotperp<Pt, Unit>(eprev, enext);
        double dpd = double(dp);
        
        if(std::abs(dp) > eps) 
            sl::addVertex(ret, *it);
        
        eprev = enext;
        if (++itx == sl::cend(sh)) itx = sl::cbegin(sh);
        ++it;
    }
    
    return ret;
}

// So I want to determine the extreme points of the polygon given a certain
// unnormalized axis vector as the positive x axis directon.
template <class Poly, 
          class Pt   = TPoint<Poly>,
          class Unit = TCoord<Pt>,
          class It = typename TContour<Poly>::const_iterator> 
std::array<It, 4> minRect(const Poly& p, const Pt& x_axis)
{
    std::array<It, 4> ret;
    
    Pt y_axis = -perp(x_axis);
    
    
    return ret;
}

// The area of the bounding rectangle with the axis dir and support vertices
template<class Pt, class Unit = TCoord<Pt>> 
inline TRational<Unit> rectarea(const Pt& w, // the axis
                                const Pt& vb, const Pt& vr, 
                                const Pt& vt, const Pt& vl) 
{
    Unit a = dot<Pt, Unit>(w, vr - vl); 
    Unit b = dot<Pt, Unit>(-perp(w), vt - vb);
    TRational<Unit> m(a, magnsq<Pt, Unit>(w));
    m = m * b;
    return m;
};

template<class Pt> 
inline double frectarea(const Pt& w, // the axis
                        const Pt& vb, const Pt& vr, 
                        const Pt& vt, const Pt& vl) 
{
    double a = dot<Pt, double>(w, vr - vl); 
    double b = dot<Pt, double>(-perp(w), vt - vb);
    double d = a / magnsq<Pt, double>(w);
    return d * b;
};

template<class Pt, 
         class Unit = TCoord<Pt>, 
         class It = typename std::vector<Pt>::const_iterator>
inline TRational<Unit> rectarea(const Pt& w, const std::array<It, 4>& rect)
{
    return rectarea<Pt, Unit>(w, *rect[0], *rect[1], *rect[2], *rect[3]);
}

template<class Pt, class It = typename std::vector<Pt>::const_iterator> 
inline double frectarea(const Pt& w, const std::array<It, 4>& rect) 
{
    return frectarea(w, *rect[0], *rect[1], *rect[2], *rect[3]);
};

// This function is only applicable to counter-clockwise oriented convex
// polygons where only two points can be collinear witch each other.
template <class RawShape, class Point = TPoint<RawShape>, class Unit = boost::multiprecision::int512_t> 
RotatedBox<Point, Unit> minAreaBoundingBox(const RawShape& sh) 
{
    using Coord = TCoord<Point>;
    using Iterator = typename TContour<RawShape>::const_iterator;

    // Get the first and the last vertex iterator
    auto first = sl::cbegin(sh);
    auto last = std::prev(sl::cend(sh));
    
    // Check conditions and return undefined box if input is not sane.
    if(last == first) return {};
    if(getX(*first) == getX(*last) && getY(*first) == getY(*last)) --last;
    if(last - first < 2) return {};
    
    // Cyclic iterator increment
    auto inc = [&first, &last](Iterator& it) {
       if(it == last) it = first; else ++it;
    };
    
    // Cyclic previous iterator
    auto prev = [&first, &last](Iterator it) { 
        return it == first ? last : std::prev(it); 
    };
    
    auto next = [&first, &last](Iterator it) {
        return it == last ? first : std::next(it);    
    };
    
    auto xcmp = [](const Point& v1, const Point& v2)
    {
        auto diff = getX(v1) - getX(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getY(v1) > getY(v2);
        return diff < 0;    
    };
    
    auto ycmp = [](const Point& v1, const Point& v2)
    {
        auto diff = getY(v1) - getY(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getX(v1) < getX(v2);
        return diff < 0;    
    };
    
    // Get axis aligned polygon extremes, used as starting positions
    auto minX = std::min_element(sl::cbegin(sh), sl::cend(sh), [](const Point& v1, const Point& v2)
    {
        auto diff = getX(v1) - getX(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getY(v1) < getY(v2);
        return diff < 0;    
    });
    auto maxX = std::min_element(sl::cbegin(sh), sl::cend(sh), [](const Point& v1, const Point& v2)
    {
        auto diff = getX(v1) - getX(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getY(v1) > getY(v2);
        return diff > 0;    
    });
    auto minY = std::min_element(sl::cbegin(sh), sl::cend(sh), [](const Point& v1, const Point& v2)
    {
        auto diff = getY(v1) - getY(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getX(v1) > getX(v2);
        return diff < 0;    
    });
    auto maxY = std::min_element(sl::cbegin(sh), sl::cend(sh), [](const Point& v1, const Point& v2)
    {
        auto diff = getY(v1) - getY(v2);
        if(std::abs(diff) <= Epsilon<Coord>::Value) return getX(v1) < getX(v2);
        return diff > 0;    
    });
    
    using Rational = TRational<Unit>;
    
    // Update the vertices defining the bounding rectangle. The rectangle with
    // the smallest rotation is selected and the supporting vertices are 
    // returned in the 'vertices' argument.
    auto update = [&sh, &next, &inc]
            (const Point& w, std::array<Iterator, 4>& rect) 
    {
        Iterator B = rect[0], Bn = next(B);
        Iterator R = rect[1], Rn = next(R);
        Iterator T = rect[2], Tn = next(T);
        Iterator L = rect[3], Ln = next(L);
        
        Point b = *Bn - *B, r = *Rn - *R, t = *Tn - *T, l = *Ln - *L;
        Point pw = perp(w);
        
        Unit dotwpb = dot<Point, Unit>(w, b),  dotwpr = dot<Point, Unit>(-pw, r);
        Unit dotwpt = dot<Point, Unit>(-w, t), dotwpl = dot<Point, Unit>(pw, l);
        Unit dw = magnsq<Point, Unit>(w);
        
        std::array<Rational, 4> angles;
        angles[0] = dotwpb * Rational(dotwpb, magnsq<Point, Unit>(b));
        angles[1] = dotwpr * Rational(dotwpr, magnsq<Point, Unit>(r));
        angles[2] = dotwpt * Rational(dotwpt, magnsq<Point, Unit>(t));
        angles[3] = dotwpl * Rational(dotwpl, magnsq<Point, Unit>(l));
        
        using AngleIndex = std::pair<Rational, size_t>;
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
    //Rational minarea((getX(*maxX) - getX(*minX)) * (getY(*maxY) - getY(*minY)));
    Rational minarea(Unit(getX(*maxX) - getX(*minX)) * (getY(*maxY) - getY(*minY)));
    
    std::array<Iterator, 4> rect = {minY, maxX, maxY, minX};
    
    // We will examine edge count + 1 bounding boxes. The one additional is 
    // the initial axis aligned bounding box
    size_t c = 0, count = sl::contourVertexCount(sh);
    std::vector<bool> edgemask(count, false);
    
    while(c++ < count) 
    {   
        // Update the support vertices, if cannot be updated, break the cycle.
        if(! update(w, rect)) break;
        
        size_t eidx = size_t(rect[0] - sl::cbegin(sh));
        
        if(edgemask[eidx]) break;
        edgemask[eidx] = true;
                
        // get the unnormalized direction vector
        w = *rect[0] - *prev(rect[0]);
        
        // get the area of the rotated rectangle
        Rational rarea = rectarea<Point, Unit>(w, rect);
        double areacheck = frectarea(w, rect);
        
        // Update min area and the direction of the min bounding box;
        if(rarea <= minarea) { w_min = w; minarea = rarea; }
    }
    
//    c = 0;
//    for(bool em : edgemask) {
//        if(!em) std::cout << "Unvisited edge: " << c << std::endl;
//        c++;
//    }
    
    Unit a = dot<Point, Unit>(w_min, *rect[1] - *rect[3]);
    Unit b = dot<Point, Unit>(-perp(w_min), *rect[2] - *rect[0]);
    RotatedBox<Point, Unit> bb(w_min, a, b, boost::rational_cast<long double>(minarea));
    
    return bb;
}

template <class RawShape> Radians minAreaBoundingBoxRotation(const RawShape& sh)
{
    return minAreaBoundingBox(sh).angleToX();
}


}

#endif // ROTCALIPERS_HPP
