#include <iostream>
#include <fstream>
#include <string>

//#define DEBUG_EXPORT_NFP

#include <libnest2d.h>
#include <libnest2d/geometries_io.hpp>

#include "printer_parts.h"
#include "benchmark.h"
#include "svgtools.hpp"
#include <libnest2d/optimizer.hpp>

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

    auto input = stegoParts();
    const int SCALE = 1000000;

    Box bin(250*SCALE, 210*SCALE);

    Coord min_obj_distance = 6*SCALE;

    NfpPlacer::Config pconf;
    pconf.alignment = NfpPlacer::Config::Alignment::BOTTOM_LEFT;
    pconf.rotations = {0.0, Pi/2.0, Pi, 3*Pi/2 };
    Arranger<NfpPlacer, DJDHeuristic> arrange(bin, min_obj_distance, pconf);

    arrange.progressIndicator([&](unsigned r){
        svg::SVGWriter::Config conf;
        conf.mm_in_coord_units = SCALE;
        svg::SVGWriter svgw(conf);
        svgw.setSize(bin);
        svgw.writePackGroup(arrange.lastResult());
        svgw.save("debout");
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
    svgw.save("out");
}

int main(void /*int argc, char **argv*/) {
    Optimizer opt;
    opt.optimize<char, double>({'a', 'z'}, {0.0, 1.0}, [](std::tuple<char, double>){ return 0.0; });
//    arrangeRectangles();
//    findDegenerateCase();
    return EXIT_SUCCESS;
}
