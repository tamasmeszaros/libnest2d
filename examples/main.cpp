#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>

#include <libnest2d/libnest2d.hpp>

#include "../tools/printer_parts.hpp"
#include "../tools/svgtools.hpp"

using namespace libnest2d;

static const std::vector<Item>& _parts(std::vector<Item>& ret, const TestData& data)
{
    if(ret.empty()) {
        ret.reserve(data.size());
        for(auto& inp : data)
            ret.emplace_back(inp);
    }

    return ret;
}

static const std::vector<Item>& prusaParts() {
    static std::vector<Item> ret;
    return _parts(ret, PRINTER_PART_POLYGONS);
}

int main(void /*int argc, char **argv*/) {
    
    std::vector<Item> input = prusaParts();
    
    size_t bins = libnest2d::nest(input, Box(mm(250), mm(210)), 0, {},
                                  ProgressFunction{[](unsigned cnt) {
          std::cout << "parts left: " << cnt << std::endl;
        }});
    
    PackGroup pgrp(bins);
    
    for (Item &itm : input) {
        if (itm.binId() >= 0) pgrp[size_t(itm.binId())].emplace_back(itm);
    }

    using SVGWriter = libnest2d::svg::SVGWriter<PolygonImpl>;
    SVGWriter::Config conf;
    conf.mm_in_coord_units = mm();
    SVGWriter svgw(conf);
    svgw.setSize(Box(mm(250), mm(210)));
    svgw.writePackGroup(pgrp);
    svgw.save("out");

    return EXIT_SUCCESS;
}
