#include <iostream>
#include <fstream>
#include <string>

#include <libnest2d.h>
#include <libnest2d/geometries_io.hpp>

#include "printer_parts.h"
#include "benchmark.h"
#include "svgtools.hpp"

namespace {
using namespace libnest2d;
using ItemGroup = std::vector<std::reference_wrapper<Item>>;
//using PackGroup = std::vector<ItemGroup>;

template< int SCALE, class Bin>
void exportSVG(ItemGroup& result, const Bin& bin, int idx) {

    std::string loc = "out";

    static std::string svg_header =
R"raw(<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.0//EN" "http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd">
<svg height="500" width="500" xmlns="http://www.w3.org/2000/svg" xmlns:svg="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink">
)raw";

    int i = idx;
    auto r = result;
//    for(auto r : result) {
        std::fstream out(loc + std::to_string(i) + ".svg", std::fstream::out);
        if(out.is_open()) {

            out << svg_header;

            Item rbin( Rectangle(bin.width(), bin.height()) );
            out << ShapeLike::serialize<Formats::SVG>(rbin.rawShape(),
                                                      1.0/SCALE)
                << std::endl;

            for(Item& sh : r) {
                Item tsh(sh.transformedShape());
                out << ShapeLike::serialize<Formats::SVG>(tsh.rawShape(),
                                                          1.0/SCALE)
                    << std::endl;
            }

            out << "\n</svg>" << std::endl;
        }
        out.close();

//        i++;
//    }
}
}

std::vector<libnest2d::Item>& _parts(std::vector<libnest2d::Item>& ret,
                                     const TestData& data)
{
    if(ret.empty()) {
        ret.reserve(data.size());
        for(auto& inp : data)
            ret.emplace_back(inp);
    }

    return ret;
}

std::vector<libnest2d::Item>& prusaParts() {
    static std::vector<Item> ret;
    return _parts(ret, PRINTER_PART_POLYGONS);
}

std::vector<libnest2d::Item>& stegoParts() {
    static std::vector<Item> ret;
    return _parts(ret, STEGOSAUR_POLYGONS);
}

void findDegenerateCase() {
    using namespace libnest2d;

    auto& input = prusaParts();

    auto scaler = [](Item& item) {
        for(unsigned i = 0; i < item.vertexCount(); i++) {
            auto v = item.vertex(i);
            setX(v, 100*getX(v)); setY(v, 100*getY(v));
            item.setVertex(i, v);
        }
    };

    auto cmp = [](const Item& t1, const Item& t2) {
        return t1.area() > t2.area();
    };

    std::for_each(input.begin(), input.end(), scaler);

    std::sort(input.begin(), input.end(), cmp);

    Box bin(210*100, 250*100);
    BottomLeftPlacer placer(bin);

    auto it = input.begin();
    auto next = it;
    int i = 0;
    while(it != input.end() && ++next != input.end()) {
        placer.pack(*it);
        placer.pack(*next);

        auto result = placer.getItems();
        bool valid = true;

        if(result.size() == 2) {
            Item& r1 = result[0];
            Item& r2 = result[1];
            valid = !Item::intersects(r1, r2) || Item::touches(r1, r2);
            valid = (valid && !r1.isInside(r2) && !r2.isInside(r1));
            if(!valid) {
                std::cout << "error index: " << i << std::endl;
                exportSVG<100>(result, bin, i);
            }
        } else {
            std::cout << "something went terribly wrong!" << std::endl;
        }


        placer.clearItems();
        it++;
        i++;
    }
}

void arrangeRectangles() {
    using namespace libnest2d;

    auto input = stegoParts();

    const int SCALE = 1000000;

    Box bin(210*SCALE, 250*SCALE);

    Coord min_obj_distance = 6*SCALE;

    Arranger<NfpPlacer, DJDHeuristic> arrange(bin, min_obj_distance);

    arrange.progressIndicator([](unsigned r){
        std::cout << "Remaining items: " << r << std::endl;
    });

    Benchmark bench;

    bench.start();
    auto result = arrange(input.begin(),
                          input.end());

    bench.stop();

    std::cout << bench.getElapsedSec() << std::endl;

    for(auto& it : input) {
        auto ret = ShapeLike::isValid(it.transformedShape());
        std::cout << ret.second << std::endl;
    }

    svg::SVGWriter::Config conf;
    conf.mm_in_coord_units = SCALE;
    svg::SVGWriter svgw(conf);
    svgw.setSize(bin);
//    exportSVG<SCALE>(result, bin);
    svgw.writePackGroup(result);
    svgw.save("out");
}

int main(void /*int argc, char **argv*/) {
    arrangeRectangles();
//    findDegenerateCase();
    return EXIT_SUCCESS;
}
