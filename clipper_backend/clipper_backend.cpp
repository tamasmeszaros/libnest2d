#include "clipper_backend.hpp"

namespace binpack2d {

HoleCache holeCache;

template<>
std::string ShapeLike::toString(const PolygonImpl& sh)
{
   std::stringstream ss;

   for(auto p : sh.Contour) {
       ss << p.X << " " << p.Y << "\n";
   }

   return ss.str();
}

template<> PolygonImpl ShapeLike::create( std::initializer_list< PointImpl > il)
{
    PolygonImpl p;
    p.Contour = il;

    // Expecting that the coordinate system Y axis is positive in upwards
    // direction
    if(ClipperLib::Orientation(p.Contour)) {
        // Not clockwise then reverse the b*tch
        ClipperLib::ReversePath(p.Contour);
    }

    return p;
}

}

