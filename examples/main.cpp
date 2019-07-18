#include <iostream>
#include <string>
#include <fstream>
#include <cstdint>
//#define DEBUG_EXPORT_NFP

#include <libnest2d.h>

#include "../tests/printer_parts.h"
#include "../tools/benchmark.h"
#include "../tools/svgtools.hpp"

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
}}
#endif

using namespace libnest2d;

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

int main(void /*int argc, char **argv*/) {
    
    std::vector<Item> input = prusaParts();
    libnest2d::nest(input, Box(250000000, 210000000),
                                  [](unsigned cnt) {
        std::cout << "parts left: " << cnt << std::endl;
    });

    using SVGWriter = libnest2d::svg::SVGWriter<PolygonImpl>;
    SVGWriter::Config conf;
    conf.mm_in_coord_units = 1000000;
    SVGWriter svgw(conf);
    svgw.setSize(Box(250000000, 210000000));
    svgw.writeItems(input.begin(), input.end());
    svgw.save("out");

    return EXIT_SUCCESS;
}
