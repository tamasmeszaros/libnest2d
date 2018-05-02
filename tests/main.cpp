#include <iostream>
#include <fstream>
#include <string>

#include <binpack2d.h>
#include <binpack2d/geometries_io.hpp>

#include "printer_parts.h"

namespace {
using namespace binpack2d;
using ItemGroup = std::vector<std::reference_wrapper<Item>>;
//using PackGroup = std::vector<ItemGroup>;

template<int SCALE, class Bin >
void exportSVG(PackGroup& result, const Bin& bin) {

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
            for(auto&v : rbin) {
                setY(v, -getY(v)/SCALE + 500 );
                setX(v, getX(v)/SCALE);
            }
            out << ShapeLike::serialize<Formats::SVG>(rbin.rawShape()) << std::endl;
            for(Item& sh : r) {
                Item tsh = sh.transformedShape();
                for(auto&v : tsh) {
                    setY(v, -getY(v)/SCALE + 500);
                    setX(v, getX(v)/SCALE);
                }
                out << ShapeLike::serialize<Formats::SVG>(tsh.rawShape()) << std::endl;
            }
            out << "\n</svg>" << std::endl;
        }
        out.close();

        i++;
    }
}

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
            Rectangle rbin(bin.width(), bin.height());
            for(auto&v : rbin) {
                setY(v, -getY(v)/SCALE + 500 );
                setX(v, getX(v)/SCALE);
            }
            out << ShapeLike::serialize<Formats::SVG>(rbin.rawShape()) << std::endl;
            for(Item& sh : r) {
                Item tsh = sh.transformedShape();
                for(auto&v : tsh) {
                    setY(v, -getY(v)/SCALE + 500);
                    setX(v, getX(v)/SCALE);
                }
                out << ShapeLike::serialize<Formats::SVG>(tsh.rawShape()) << std::endl;
            }
            out << "\n</svg>" << std::endl;
        }
        out.close();

//        i++;
//    }
}
}


void findDegenerateCase() {
    using namespace binpack2d;

    auto input = PRINTER_PART_POLYGONS;

    auto scaler = [](Item& item) {
        for(auto& v : item) { setX(v, 100*getX(v)); setY(v, 100*getY(v));}
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
    using namespace binpack2d;

    BottomLeftPlacer::Config config;
    config.min_obj_distance = 2;



//    std::vector<PolygonImpl> input;
    auto input = PRINTER_PART_POLYGONS;

    const int SCALE = 1000;
    auto scaler = [&SCALE](Item& item) {
        for(auto& v : item) { setX(v, SCALE*getX(v)); setY(v, SCALE*getY(v));}
    };

    std::for_each(input.begin(), input.end(), scaler);

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
//        {20, 20},
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

    Box bin(SCALE*210, SCALE*250);
    DJDArranger arrange(bin, config /*{.min_obj_distance = 10}*/ );

    for(auto& it : input) {
        auto ret = ShapeLike::isValid(it.rawShape());
        std::cout << ret.second << std::endl;
    }

    auto result = arrange(input.begin(),
                          input.end());

    exportSVG<SCALE>(result, bin);

}

int main(void /*int argc, char **argv*/) {
    arrangeRectangles();
//    findDegenerateCase();
    return EXIT_SUCCESS;
}
