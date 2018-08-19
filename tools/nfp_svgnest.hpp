#ifndef NFP_SVGNEST_HPP
#define NFP_SVGNEST_HPP

#include <limits>

#include <libnest2d/geometry_traits_nfp.hpp>

namespace libnest2d {

namespace __svgnest {

using std::sqrt;
using std::min;
using std::max;
using std::abs;
using std::isnan;

template<class Coord> struct _Tol {
    static const BP2D_CONSTEXPR long long Value = 1000000;
};

template<class S> struct _alg {
    using Cntr = TContour<S>;
    using Point = TPoint<S>;
    using Coord = TCoord<Point>;
    using Shapes = nfp::Shapes<S>;

#define TOL _Tol<Coord>::Value
#define dNAN std::nan("")

    struct Vector {
        Coord x, y;
        Vector(Coord X, Coord Y): x(X), y(Y) {}
        Vector(const Point& p): x(getX(p)), y(getY(p)) {}
        operator Point() const { return {x, y}; }
        Vector& operator=(const Point& p) {
            x = getX(p), y = getY(p); return *this;
        }
        Vector(std::initializer_list<Coord> il):
            x(*il.begin()), y(*std::next(il.begin())) {}
    };


    inline static bool _almostEqual(Coord a, Coord b,
                                    Coord tolerance = _Tol<S>::Value)
    {
        return std::abs(a - b) < tolerance;
    };

    static Point _normalizeVector(const Vector& v) {
        if(_almostEqual(v.x*v.x + v.y*v.y, Coord(1))){
            return Point(v); // given vector was already a unit vector
        }
        auto len = sqrt(v.x*v.x + v.y*v.y);
        auto inverse = 1/len;

        return { Coord(v.x*inverse), Coord(v.y*inverse) };
    };

    static double pointDistance( const Vector& p,
                                 const Vector& s1,
                                 const Vector& s2,
                                 Vector normal,
                                 bool infinite = false)
    {
        normal = _normalizeVector(normal);

        Vector dir = {
            normal.y,
            -normal.x
        };

        auto pdot = p.x*dir.x + p.y*dir.y;
        auto s1dot = s1.x*dir.x + s1.y*dir.y;
        auto s2dot = s2.x*dir.x + s2.y*dir.y;

        auto pdotnorm = p.x*normal.x + p.y*normal.y;
        auto s1dotnorm = s1.x*normal.x + s1.y*normal.y;
        auto s2dotnorm = s2.x*normal.x + s2.y*normal.y;

        if(!infinite){
            if (((pdot<s1dot || _almostEqual(pdot, s1dot)) &&
                 (pdot<s2dot || _almostEqual(pdot, s2dot))) ||
                    ((pdot>s1dot || _almostEqual(pdot, s1dot)) &&
                     (pdot>s2dot || _almostEqual(pdot, s2dot))))
            {
                // dot doesn't collide with segment,
                // or lies directly on the vertex
                return dNAN;
            }
            if ((_almostEqual(pdot, s1dot) && _almostEqual(pdot, s2dot)) &&
                    (pdotnorm>s1dotnorm && pdotnorm>s2dotnorm))
            {
                return double(min(pdotnorm - s1dotnorm, pdotnorm - s2dotnorm));
            }
            if ((_almostEqual(pdot, s1dot) && _almostEqual(pdot, s2dot)) &&
                    (pdotnorm<s1dotnorm && pdotnorm<s2dotnorm)){
                return double(-min(s1dotnorm-pdotnorm, s2dotnorm-pdotnorm));
            }
        }

        return -(pdotnorm - s1dotnorm + (s1dotnorm - s2dotnorm)*(s1dot - pdot)
                 / double(s1dot - s2dot));
    };

    static double segmentDistance( const Vector& A,
                                   const Vector& B,
                                   const Vector& E,
                                   const Vector& F,
                                   Vector direction)
    {
        Vector normal = {
            direction.y,
            -direction.x
        };

        Vector reverse = {
            -direction.x,
            -direction.y
        };

        auto dotA = A.x*normal.x + A.y*normal.y;
        auto dotB = B.x*normal.x + B.y*normal.y;
        auto dotE = E.x*normal.x + E.y*normal.y;
        auto dotF = F.x*normal.x + F.y*normal.y;

        auto crossA = A.x*direction.x + A.y*direction.y;
        auto crossB = B.x*direction.x + B.y*direction.y;
        auto crossE = E.x*direction.x + E.y*direction.y;
        auto crossF = F.x*direction.x + F.y*direction.y;

        auto crossABmin = min(crossA, crossB);
        auto crossABmax = max(crossA, crossB);

        auto crossEFmax = max(crossE, crossF);
        auto crossEFmin = min(crossE, crossF);

        auto ABmin = min(dotA, dotB);
        auto ABmax = max(dotA, dotB);

        auto EFmax = max(dotE, dotF);
        auto EFmin = min(dotE, dotF);

        // segments that will merely touch at one point
        if(_almostEqual(ABmax, EFmin,TOL) || _almostEqual(ABmin, EFmax,TOL)){
            return dNAN;
        }
        // segments miss eachother completely
        if(ABmax < EFmin || ABmin > EFmax){
            return dNAN;
        }

        double overlap = 0;

        if((ABmax > EFmax && ABmin < EFmin) || (EFmax > ABmax && EFmin < ABmin))
        {
            overlap = 1;
        }
        else{
            auto minMax = min(ABmax, EFmax);
            auto maxMin = max(ABmin, EFmin);

            auto maxMax = max(ABmax, EFmax);
            auto minMin = min(ABmin, EFmin);

            overlap = (minMax-maxMin)/(maxMax-minMin);
        }

        auto crossABE = (E.y - A.y) * (B.x - A.x) - (E.x - A.x) * (B.y - A.y);
        auto crossABF = (F.y - A.y) * (B.x - A.x) - (F.x - A.x) * (B.y - A.y);

        // lines are colinear
        if(_almostEqual(crossABE,0) && _almostEqual(crossABF,0)){

            Vector ABnorm = {B.y-A.y, A.x-B.x};
            Vector EFnorm = {F.y-E.y, E.x-F.x};

            auto ABnormlength = sqrt(ABnorm.x*ABnorm.x + ABnorm.y*ABnorm.y);
            ABnorm.x /= ABnormlength;
            ABnorm.y /= ABnormlength;

            auto EFnormlength = sqrt(EFnorm.x*EFnorm.x + EFnorm.y*EFnorm.y);
            EFnorm.x /= EFnormlength;
            EFnorm.y /= EFnormlength;

            // segment normals must point in opposite directions
            if(abs(ABnorm.y * EFnorm.x - ABnorm.x * EFnorm.y) < TOL &&
                    ABnorm.y * EFnorm.y + ABnorm.x * EFnorm.x < 0){
                // normal of AB segment must point in same direction as
                // given direction vector
                auto normdot = ABnorm.y * direction.y + ABnorm.x * direction.x;
                // the segments merely slide along eachother
                if(_almostEqual(normdot,0, TOL)){
                    return dNAN;
                }
                if(normdot < 0){
                    return 0.0;
                }
            }
            return dNAN;
        }

        std::vector<double> distances; distances.reserve(10);

        // coincident points
        if(_almostEqual(dotA, dotE)){
            distances.emplace_back(crossA-crossE);
        }
        else if(_almostEqual(dotA, dotF)){
            distances.emplace_back(crossA-crossF);
        }
        else if(dotA > EFmin && dotA < EFmax){
            auto d = pointDistance(A,E,F,reverse);
            if(!isnan(d) && _almostEqual(d, 0))
            { //  A currently touches EF, but AB is moving away from EF
                auto dB = pointDistance(B,E,F,reverse,true);
                if(dB < 0 || _almostEqual(dB*overlap,0)){
                    d = dNAN;
                }
            }
            if(isnan(d)){
                distances.emplace_back(d);
            }
        }

        if(_almostEqual(dotB, dotE)){
            distances.emplace_back(crossB-crossE);
        }
        else if(_almostEqual(dotB, dotF)){
            distances.emplace_back(crossB-crossF);
        }
        else if(dotB > EFmin && dotB < EFmax){
            auto d = pointDistance(B,E,F,reverse);

            if(!isnan(d) && _almostEqual(d, 0))
            { // crossA>crossB A currently touches EF, but AB is moving away from EF
                double dA = pointDistance(A,E,F,reverse,true);
                if(dA < 0 || _almostEqual(dA*overlap,0)){
                    d = dNAN;
                }
            }
            if(!isnan(d)){
                distances.emplace_back(d);
            }
        }

        if(dotE > ABmin && dotE < ABmax){
            auto d = pointDistance(E,A,B,direction);
            if(!isnan(d) && _almostEqual(d, 0))
            { // crossF<crossE A currently touches EF, but AB is moving away from EF
                auto dF = pointDistance(F,A,B,direction, true);
                if(dF < 0 || _almostEqual(dF*overlap,0)){
                    d = dNAN;
                }
            }
            if(!isnan(d)){
                distances.emplace_back(d);
            }
        }

        if(dotF > ABmin && dotF < ABmax){
            auto d = pointDistance(F,A,B,direction);
            if(!isnan(d) && _almostEqual(d, 0))
            { // && crossE<crossF A currently touches EF,
              // but AB is moving away from EF
                auto dE = pointDistance(E,A,B,direction, true);
                if(dE < 0 || _almostEqual(dE*overlap,0)){
                    d = dNAN;
                }
            }
            if(!isnan(d)){
                distances.emplace_back(d);
            }
        }

        if(distances.empty()){
            return dNAN;
        }

        return *std::min_element(distances.begin(), distances.end());
    };

    static double polygonSlideDistance( const Cntr& A,
                                        const Cntr& B,
                                        Vector direction,
                                        bool ignoreNegative)
    {
        Cntr A1, A2, B1, B2;
        Coord Aoffsetx = 0, Aoffsety = 0, Boffsetx = 0, Boffsety = 0;
        auto x = [](const Point& p) { return getX(p); };
        auto y = [](const Point& p) { return getY(p); };

        auto& edgeA = A;
        auto& edgeB = B;

        double distance = dNAN, d = dNAN;

        Vector dir = _normalizeVector(direction);

        Vector normal = {
            dir.y,
            -dir.x
        };

        Vector reverse = {
            -dir.x,
            -dir.y,
        };

        for(auto i = 0; i < edgeB.size() - 1; i++){
            for(auto j = 0; j < edgeA.size() - 1; j++){
                A1 = {x(edgeA[j]),   y(edgeA[j]) };
                A2 = {x(edgeA[j+1]), y(edgeA[j+1]) };
                B1 = {x(edgeB[i]),   y(edgeB[i]) };
                B2 = {x(edgeB[i+1]), y(edgeB[i+1]) };

                if((_almostEqual(A1.x, A2.x) && _almostEqual(A1.y, A2.y)) ||
                   (_almostEqual(B1.x, B2.x) && _almostEqual(B1.y, B2.y))){
                    continue; // ignore extremely small lines
                }

                d = segmentDistance(A1, A2, B1, B2, dir);

                if(!isnan(d) && (isnan(distance) || d < distance)){
                    if(!ignoreNegative || d > 0 || _almostEqual(d, 0)){
                        distance = d;
                    }
                }
            }
        }
        return distance;
    };

    static double polygonProjectionDistance(const Cntr& A,
                                            const Cntr& B,
                                            Vector direction)
    {
        Coord Aoffsetx = 0, Aoffsety = 0, Boffsetx = 0, Boffsety = 0;
        auto x = [](const Point& p) { return getX(p); };
        auto y = [](const Point& p) { return getY(p); };

        // close the loop for polygons
        /*if(A[0] != A[A.length-1]){
            A.push(A[0]);
        }

        if(B[0] != B[B.length-1]){
            B.push(B[0]);
        }*/

        auto& edgeA = A;
        auto& edgeB = B;

        double distance = dNAN, d;
        Vector p, s1, s2;

        for(auto i = 0; i < edgeB.size(); i++) {
            // the shortest/most negative projection of B onto A
            double minprojection = dNAN;
            double minp = dNAN;
            for(auto j = 0; j < edgeA.size() - 1; j++){
                p =  {x(edgeB[i]), y(edgeB[i]) };
                s1 = {x(edgeA[j]), y(edgeA[j]) };
                s2 = {x(edgeA[j+1]), y(edgeA[j+1]) };

                if(abs((s2.y-s1.y) * direction.x -
                       (s2.x-s1.x) * direction.y) < TOL) continue;

                // project point, ignore edge boundaries
                d = pointDistance(p, s1, s2, direction);

                if(!isnan(d) && (isnan(minprojection) || d < minprojection)) {
                    minprojection = d;
                    minp = p;
                }
            }

            if(!isnan(minprojection) && (isnan(distance) ||
                                         minprojection > distance)){
                distance = minprojection;
            }
        }

        return distance;
    }

    static Vector searchStartPoint( const Cntr& A,
                                    const Cntr& B,
                                    const std::vector<bool>& Amarks,
                                    const std::vector<bool>& Bmarks,
                                    bool inside,
                                    const S& NFP = S())
    {
        // clone arrays
//        A = A.slice(0);
//        B = B.slice(0);

//        // close the loop for polygons
//        if(A[0] != A[A.length-1]){
//            A.push(A[0]);
//        }

//        if(B[0] != B[B.length-1]){
//            B.push(B[0]);
//        }

//        for(var i=0; i<A.length-1; i++){
//            if(!A[i].marked){
//                A[i].marked = true;
//                for(var j=0; j<B.length; j++){
//                    B.offsetx = A[i].x - B[j].x;
//                    B.offsety = A[i].y - B[j].y;

//                    var Binside = null;
//                    for(var k=0; k<B.length; k++){
//                        var inpoly = this.pointInPolygon({x: B[k].x + B.offsetx, y: B[k].y + B.offsety}, A);
//                        if(inpoly !== null){
//                            Binside = inpoly;
//                            break;
//                        }
//                    }

//                    if(Binside === null){ // A and B are the same
//                        return null;
//                    }

//                    var startPoint = {x: B.offsetx, y: B.offsety};
//                    if(((Binside && inside) || (!Binside && !inside)) && !this.intersect(A,B) && !inNfp(startPoint, NFP)){
//                        return startPoint;
//                    }

//                    // slide B along vector
//                    var vx = A[i+1].x - A[i].x;
//                    var vy = A[i+1].y - A[i].y;

//                    var d1 = this.polygonProjectionDistance(A,B,{x: vx, y: vy});
//                    var d2 = this.polygonProjectionDistance(B,A,{x: -vx, y: -vy});

//                    var d = null;

//                    // todo: clean this up
//                    if(d1 === null && d2 === null){
//                        // nothin
//                    }
//                    else if(d1 === null){
//                        d = d2;
//                    }
//                    else if(d2 === null){
//                        d = d1;
//                    }
//                    else{
//                        d = Math.min(d1,d2);
//                    }

//                    // only slide until no longer negative
//                    // todo: clean this up
//                    if(d !== null && !_almostEqual(d,0) && d > 0){

//                    }
//                    else{
//                        continue;
//                    }

//                    var vd2 = vx*vx + vy*vy;

//                    if(d*d < vd2 && !_almostEqual(d*d, vd2)){
//                        var vd = Math.sqrt(vx*vx + vy*vy);
//                        vx *= d/vd;
//                        vy *= d/vd;
//                    }

//                    B.offsetx += vx;
//                    B.offsety += vy;

//                    for(k=0; k<B.length; k++){
//                        var inpoly = this.pointInPolygon({x: B[k].x + B.offsetx, y: B[k].y + B.offsety}, A);
//                        if(inpoly !== null){
//                            Binside = inpoly;
//                            break;
//                        }
//                    }
//                    startPoint = {x: B.offsetx, y: B.offsety};
//                    if(((Binside && inside) || (!Binside && !inside)) && !this.intersect(A,B) && !inNfp(startPoint, NFP)){
//                        return startPoint;
//                    }
//                }
//            }
//        }

//        // returns true if point already exists in the given nfp
//        function inNfp(p, nfp){
//            if(!nfp || nfp.length == 0){
//                return false;
//            }

//            for(var i=0; i<nfp.length; i++){
//                for(var j=0; j<nfp[i].length; j++){
//                    if(_almostEqual(p.x, nfp[i][j].x) && _almostEqual(p.y, nfp[i][j].y)){
//                        return true;
//                    }
//                }
//            }

//            return false;
//        }

//        return null;
        return Vector(0, 0);
    }

    static S noFitPolygon(const Cntr& A,
                          const Cntr& B,
                          bool inside,
                          bool searchEdges)
    {
        if(A.size() < 3 || B.size() < 3) {
            throw GeometryException(GeomErr::NFP);
            return S();
        }

        auto x = [](const Point& p) { return getX(p); };
        auto y = [](const Point& p) { return getY(p); };

//        A.offsetx = 0;
//        A.offsety = 0;

        unsigned i = 0, j = 0;

        auto minA = y(A[0]);
        unsigned minAindex = 0;

        auto maxB = y(B[0]);
        unsigned maxBindex = 0;

        std::vector<bool> Amarks(A.size(), false);
        std::vector<bool> Bmarks(B.size(), false);

        for(i = 1; i < A.size(); i++){
            Amarks[i] = false;
            if(y(A[i]) < minA){
                minA = y(A[i]);
                minAindex = i;
            }
        }

        for(i = 1; i < B.size(); i++){
            Bmarks[i] = false;
            if(y(B[i]) > maxB){
                maxB = y(B[i]);
                maxBindex = i;
            }
        }

        if(!inside){
            // shift B such that the bottom-most point of B is at the top-most
            // point of A. This guarantees an initial placement with no
            // intersections
            Vector startpoint = {
                x(A[minAindex]) - x(B[maxBindex]),
                y(A[minAindex]) - y(B[maxBindex])
            };
        }
        else{
            // no reliable heuristic for inside
            auto startpoint = searchStartPoint(A, B, Amarks, Bmarks, true);
        }

//        var NFPlist = [];

//        while(startpoint !== null){

//            B.offsetx = startpoint.x;
//            B.offsety = startpoint.y;

//            // maintain a list of touching points/edges
//            var touching;

//            var prevvector = null; // keep track of previous vector
//            var NFP = [{
//                x: B[0].x+B.offsetx,
//                y: B[0].y+B.offsety
//            }];

//            var referencex = B[0].x+B.offsetx;
//            var referencey = B[0].y+B.offsety;
//            var startx = referencex;
//            var starty = referencey;
//            var counter = 0;

//            while(counter < 10*(A.length + B.length)){ // sanity check, prevent infinite loop
//                touching = [];
//                // find touching vertices/edges
//                for(i=0; i<A.length; i++){
//                    var nexti = (i==A.length-1) ? 0 : i+1;
//                    for(j=0; j<B.length; j++){
//                        var nextj = (j==B.length-1) ? 0 : j+1;
//                        if(_almostEqual(A[i].x, B[j].x+B.offsetx) && _almostEqual(A[i].y, B[j].y+B.offsety)){
//                            touching.push({	type: 0, A: i, B: j });
//                        }
//                        else if(_onSegment(A[i],A[nexti],{x: B[j].x+B.offsetx, y: B[j].y + B.offsety})){
//                            touching.push({	type: 1, A: nexti, B: j });
//                        }
//                        else if(_onSegment({x: B[j].x+B.offsetx, y: B[j].y + B.offsety},{x: B[nextj].x+B.offsetx, y: B[nextj].y + B.offsety},A[i])){
//                            touching.push({	type: 2, A: i, B: nextj });
//                        }
//                    }
//                }

//                // generate translation vectors from touching vertices/edges
//                var vectors = [];
//                for(i=0; i<touching.length; i++){
//                    var vertexA = A[touching[i].A];
//                    vertexA.marked = true;

//                    // adjacent A vertices
//                    var prevAindex = touching[i].A-1;
//                    var nextAindex = touching[i].A+1;

//                    prevAindex = (prevAindex < 0) ? A.length-1 : prevAindex; // loop
//                    nextAindex = (nextAindex >= A.length) ? 0 : nextAindex; // loop

//                    var prevA = A[prevAindex];
//                    var nextA = A[nextAindex];

//                    // adjacent B vertices
//                    var vertexB = B[touching[i].B];

//                    var prevBindex = touching[i].B-1;
//                    var nextBindex = touching[i].B+1;

//                    prevBindex = (prevBindex < 0) ? B.length-1 : prevBindex; // loop
//                    nextBindex = (nextBindex >= B.length) ? 0 : nextBindex; // loop

//                    var prevB = B[prevBindex];
//                    var nextB = B[nextBindex];

//                    if(touching[i].type == 0){

//                        var vA1 = {
//                            x: prevA.x-vertexA.x,
//                            y: prevA.y-vertexA.y,
//                            start: vertexA,
//                            end: prevA
//                        };

//                        var vA2 = {
//                            x: nextA.x-vertexA.x,
//                            y: nextA.y-vertexA.y,
//                            start: vertexA,
//                            end: nextA
//                        };

//                        // B vectors need to be inverted
//                        var vB1 = {
//                            x: vertexB.x-prevB.x,
//                            y: vertexB.y-prevB.y,
//                            start: prevB,
//                            end: vertexB
//                        };

//                        var vB2 = {
//                            x: vertexB.x-nextB.x,
//                            y: vertexB.y-nextB.y,
//                            start: nextB,
//                            end: vertexB
//                        };

//                        vectors.push(vA1);
//                        vectors.push(vA2);
//                        vectors.push(vB1);
//                        vectors.push(vB2);
//                    }
//                    else if(touching[i].type == 1){
//                        vectors.push({
//                            x: vertexA.x-(vertexB.x+B.offsetx),
//                            y: vertexA.y-(vertexB.y+B.offsety),
//                            start: prevA,
//                            end: vertexA
//                        });

//                        vectors.push({
//                            x: prevA.x-(vertexB.x+B.offsetx),
//                            y: prevA.y-(vertexB.y+B.offsety),
//                            start: vertexA,
//                            end: prevA
//                        });
//                    }
//                    else if(touching[i].type == 2){
//                        vectors.push({
//                            x: vertexA.x-(vertexB.x+B.offsetx),
//                            y: vertexA.y-(vertexB.y+B.offsety),
//                            start: prevB,
//                            end: vertexB
//                        });

//                        vectors.push({
//                            x: vertexA.x-(prevB.x+B.offsetx),
//                            y: vertexA.y-(prevB.y+B.offsety),
//                            start: vertexB,
//                            end: prevB
//                        });
//                    }
//                }

//                // todo: there should be a faster way to reject vectors that will cause immediate intersection. For now just check them all

//                var translate = null;
//                var maxd = 0;

//                for(i=0; i<vectors.length; i++){
//                    if(vectors[i].x == 0 && vectors[i].y == 0){
//                        continue;
//                    }

//                    // if this vector points us back to where we came from, ignore it.
//                    // ie cross product = 0, dot product < 0
//                    if(prevvector && vectors[i].y * prevvector.y + vectors[i].x * prevvector.x < 0){

//                        // compare magnitude with unit vectors
//                        var vectorlength = Math.sqrt(vectors[i].x*vectors[i].x+vectors[i].y*vectors[i].y);
//                        var unitv = {x: vectors[i].x/vectorlength, y:vectors[i].y/vectorlength};

//                        var prevlength = Math.sqrt(prevvector.x*prevvector.x+prevvector.y*prevvector.y);
//                        var prevunit = {x: prevvector.x/prevlength, y:prevvector.y/prevlength};

//                        // we need to scale down to unit vectors to normalize vector length. Could also just do a tan here
//                        if(Math.abs(unitv.y * prevunit.x - unitv.x * prevunit.y) < 0.0001){
//                            continue;
//                        }
//                    }

//                    var d = this.polygonSlideDistance(A, B, vectors[i], true);
//                    var vecd2 = vectors[i].x*vectors[i].x + vectors[i].y*vectors[i].y;

//                    if(d === null || d*d > vecd2){
//                        var vecd = Math.sqrt(vectors[i].x*vectors[i].x + vectors[i].y*vectors[i].y);
//                        d = vecd;
//                    }

//                    if(d !== null && d > maxd){
//                        maxd = d;
//                        translate = vectors[i];
//                    }
//                }


//                if(translate === null || _almostEqual(maxd, 0)){
//                    // didn't close the loop, something went wrong here
//                    NFP = null;
//                    break;
//                }

//                translate.start.marked = true;
//                translate.end.marked = true;

//                prevvector = translate;

//                // trim
//                var vlength2 = translate.x*translate.x + translate.y*translate.y;
//                if(maxd*maxd < vlength2 && !_almostEqual(maxd*maxd, vlength2)){
//                    var scale = Math.sqrt((maxd*maxd)/vlength2);
//                    translate.x *= scale;
//                    translate.y *= scale;
//                }

//                referencex += translate.x;
//                referencey += translate.y;

//                if(_almostEqual(referencex, startx) && _almostEqual(referencey, starty)){
//                    // we've made a full loop
//                    break;
//                }

//                // if A and B start on a touching horizontal line, the end point may not be the start point
//                var looped = false;
//                if(NFP.length > 0){
//                    for(i=0; i<NFP.length-1; i++){
//                        if(_almostEqual(referencex, NFP[i].x) && _almostEqual(referencey, NFP[i].y)){
//                            looped = true;
//                        }
//                    }
//                }

//                if(looped){
//                    // we've made a full loop
//                    break;
//                }

//                NFP.push({
//                    x: referencex,
//                    y: referencey
//                });

//                B.offsetx += translate.x;
//                B.offsety += translate.y;

//                counter++;
//            }

//            if(NFP && NFP.length > 0){
//                NFPlist.push(NFP);
//            }

//            if(!searchEdges){
//                // only get outer NFP or first inner NFP
//                break;
//            }

//            startpoint = this.searchStartPoint(A,B,inside,NFPlist);

//        }

//        return NFPlist;
        return S();
    }
};


template<class S>
nfp::NfpResult<S> nfpSimpleSimple(const S& stat, const S& orb) {
//    using Cntr = TContour<S>;
    using Point = TPoint<S>;
//    using Coord = TCoord<Point>;
//    using Shapes = nfp::Shapes<S>;

    namespace sl = shapelike;

    noFitPolygon(sl::getContour(stat), sl::getContour(orb), true, true);
    return {S(), Point()};
}

}
}

#endif // NFP_SVGNEST_HPP
