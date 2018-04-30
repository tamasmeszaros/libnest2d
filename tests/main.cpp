#include <iostream>
#include <fstream>
#include <string>

#include <binpack2d.h>
#include <binpack2d/geometries_io.hpp>

#include "printer_parts.h"

namespace {
using namespace binpack2d;

template<class Arranger,
         class Result = typename Arranger::PackGroup,
         class Bin = typename Arranger::BinType>
void exportSVG(Result& result, const Bin& bin) {

    std::string loc = "out";

    static std::string svg_header =
R"raw(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.0//EN" "http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">
<svg height="500" width="500" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
)raw";

    int i = 0;
    for(auto r : result) {
        std::fstream out(loc + std::to_string(i) + ".svg", std::fstream::out);
        if(out.is_open()) {
            out << svg_header;
            Rectangle rbin(bin.width(), bin.height());
            for(auto&v : rbin) setY(v, -getY(v) + 500 );
            out << ShapeLike::serialize<Formats::SVG>(rbin.rawShape()) << std::endl;
            for(Item& sh : r) {
                Item tsh = sh.transformedShape();
                for(auto&v : tsh) setY(v, -getY(v) + 500 );
                out << ShapeLike::serialize<Formats::SVG>(tsh.rawShape()) << std::endl;
            }
            out << "\n</svg>" << std::endl;
        }
        out.close();

        i++;
    }
}
}

void arrangeRectangles() {
    using namespace binpack2d;

    BottomLeftPlacer::Config config;
    config.min_obj_distance = 6;

//    std::vector<PolygonImpl> input;
    auto input = PRINTER_PART_POLYGONS;
//    std::vector<Rectangle> input = {
//        {200, 200},
//        {}

//    };
//    std::vector<Rectangle> input = {
//        {80, 80},
//        {60, 90},
//        {70, 30},
//        {80, 60},
//        {60, 60},
//        {60, 40},
//        {40, 40},
//        {10, 10},
//        {10, 10},
//        {10, 10},
//        {10, 10},
//        {10, 10},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {20, 20}
//    };

//    std::vector<Rectangle> input = {
//        {80, 80},
//        {110, 10},
//        {200, 5},
//        {80, 30},
//        {60, 90},
//        {70, 30},
//        {80, 60},
//        {60, 60},
//        {60, 40},
//        {40, 40},
//        {10, 10},
//        {10, 10},
//        {10, 10},
//        {10, 10},
//        {10, 10},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {5, 5},
//        {20, 20}
//    };

    Box bin(210, 250);
    DJDArranger arrange(bin, config /*{.min_obj_distance = 10}*/ );

    for(auto& it : input) {
        auto ret = ShapeLike::isValid(it/*.rawShape()*/);
        std::cout << ret.second << std::endl;
    }

    auto result = arrange(input.begin(),
                          input.end());

    exportSVG<DJDArranger>(result, bin);

}

int main(int argc, char **argv) {
    arrangeRectangles();
    return EXIT_SUCCESS;
}
