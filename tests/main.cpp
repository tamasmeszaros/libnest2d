#include <iostream>
#include <string>
#include <fstream>

//#define DEBUG_EXPORT_NFP

#include <libnest2d.h>
#include <libnest2d/geometries_io.hpp>

#include "printer_parts.h"
#include "benchmark.h"
#include "svgtools.hpp"
//#include <libnest2d/optimizer.hpp>
//#include <libnest2d/optimizers/simplex.hpp>

using namespace libnest2d;
using ItemGroup = std::vector<std::reference_wrapper<Item>>;

std::vector<Item>& _parts(std::vector<Item>& ret, const TestData& data)
{
    if(ret.empty()) {
        ret.reserve(data.size());
        for(auto& inp : data)
            ret.emplace_back(inp);
    }

    return ret;
}

std::vector<Item>& prusaParts() {
    static std::vector<Item> ret;
    return _parts(ret, PRINTER_PART_POLYGONS);
}

std::vector<Item>& stegoParts() {
    static std::vector<Item> ret;
    return _parts(ret, STEGOSAUR_POLYGONS);
}

void arrangeRectangles() {
    using namespace libnest2d;

    const int SCALE = 1000000;
//    std::vector<Rectangle> rects = {
//        {80*SCALE, 80*SCALE},
//        {60*SCALE, 90*SCALE},
//        {70*SCALE, 30*SCALE},
//        {80*SCALE, 60*SCALE},
//        {60*SCALE, 60*SCALE},
//        {60*SCALE, 40*SCALE},
//        {40*SCALE, 40*SCALE},
//        {10*SCALE, 10*SCALE},
//        {10*SCALE, 10*SCALE},
//        {10*SCALE, 10*SCALE},
//        {10*SCALE, 10*SCALE},
//        {10*SCALE, 10*SCALE},
//        {5*SCALE, 5*SCALE},
//        {5*SCALE, 5*SCALE},
//        {5*SCALE, 5*SCALE},
//        {5*SCALE, 5*SCALE},
//        {5*SCALE, 5*SCALE},
//        {5*SCALE, 5*SCALE},
//        {5*SCALE, 5*SCALE},
//        {20*SCALE, 20*SCALE}
//       };

    std::vector<Rectangle> rects = {
        {20*SCALE, 10*SCALE},
        {20*SCALE, 10*SCALE}
    };

    std::vector<Item> input {
        {{0, 0}, {0, 20*SCALE}, {10*SCALE, 0}, {0, 0}}
    };

//    std::vector<Item> input;
//    input.insert(input.end(), prusaParts().begin(), prusaParts().end());
//    input.insert(input.end(), stegoParts().begin(), stegoParts().end());
    input.insert(input.end(), rects.begin(), rects.end());

    Box bin(250*SCALE, 210*SCALE);

    Coord min_obj_distance = 1.5*SCALE;

    using Packer = Arranger<NfpPlacer, DJDHeuristic>;

    Packer::PlacementConfig pconf;
    pconf.alignment = NfpPlacer::Config::Alignment::BOTTOM_LEFT;
    pconf.rotations = {0.0, Pi/2.0, Pi, 3*Pi/2};
    Packer::SelectionConfig sconf;
    sconf.allow_parallel = true;
    Packer arrange(bin, min_obj_distance, pconf, sconf);

    arrange.progressIndicator([&](unsigned r){
//        svg::SVGWriter::Config conf;
//        conf.mm_in_coord_units = SCALE;
//        svg::SVGWriter svgw(conf);
//        svgw.setSize(bin);
//        svgw.writePackGroup(arrange.lastResult());
//        svgw.save("debout");
        std::cout << "Remaining items: " << r << std::endl;
    });

    Benchmark bench;

    bench.start();
    auto result = arrange.arrange(input.begin(),
                          input.end());

    bench.stop();

    std::cout << bench.getElapsedSec() << " bin count: " <<  result.size() << std::endl;

    for(auto& it : input) {
        auto ret = ShapeLike::isValid(it.transformedShape());
        std::cout << ret.second << std::endl;
    }

    svg::SVGWriter::Config conf;
    conf.mm_in_coord_units = SCALE;
    svg::SVGWriter svgw(conf);
    svgw.setSize(bin);
    svgw.writePackGroup(result);
//    std::for_each(input.begin(), input.end(), [&svgw](Item& item){ svgw.writeItem(item);});
    svgw.save("out");
}



int main(void /*int argc, char **argv*/) {
    arrangeRectangles();
//    findDegenerateCase();
    return EXIT_SUCCESS;
}
