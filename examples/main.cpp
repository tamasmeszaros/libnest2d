#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
//#define DEBUG_EXPORT_NFP

#include <libnest2d.h>

#include "../tools/printer_parts.hpp"
#include "../tools/benchmark.h"
#include "../tools/svgtools.hpp"
#include "../tools/measure.hpp"

#include <boost/rational.hpp>

//#include "tools/libnfpglue.hpp"
//#include "tools/nfp_svgnest_glue.hpp"

#if ! defined(_MSC_VER) && defined(__SIZEOF_INT128__)
namespace libnest2d { namespace nfp {
template<class RawShape>
struct NfpImpl<RawShape, NfpLevel::CONVEX_ONLY> {
    NfpResult<RawShape> operator()(const RawShape& sh, const RawShape& other)
    {
        return nfpConvexOnly<RawShape, boost::rational<__int128>>(sh, other);
    }
};
} // namespace nfp
} // namespace libnest2d
#endif

using namespace libnest2d;

namespace  {

const std::vector<Item>& _parts(std::vector<Item>& ret, const TestData& data)
{
    if(ret.empty()) {
        ret.reserve(data.size());
        for(auto& inp : data)
            ret.emplace_back(inp);
    }

    return ret;
}

const std::vector<Item>& prusaParts() {
    static std::vector<Item> ret;
    return _parts(ret, PRINTER_PART_POLYGONS);
}

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
