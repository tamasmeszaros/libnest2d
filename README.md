# Introduction

Libnest2D is a library and framework for the 2D bin packaging problem. 
Inspired from the [SVGNest](svgnest.com) Javascript library the project is  
built from scratch in C++11. The library is written with a policy that it should
be usable out of the box with a very simple interface but has to be customizable
to the very core as well. The algorithms are defined in a header only fashion 
with templated geometry types. These geometries can have custom or already 
existing implementation to avoid copying or having unnecessary dependencies.

A default backend is provided if the user of the library just wants to use it 
out of the box without additional integration. This backend is reasonably 
fast and robust, being built on top of boost geometry and the 
[polyclipping](http://www.angusj.com/delphi/clipper.php) library. Usage of 
this default backend implies the dependency on these packages but its header 
only as well.

This software is currently under construction and lacks a throughout 
documentation and some essential algorithms as well. At this stage it works well
for rectangles and convex closed polygons without considering holes and 
concavities.

Holes and non-convex polygons will be usable in the near future as well. The 
no fit polygon based placer module combined with the first fit selection 
strategy is now used in the [Slic3r](https://github.com/prusa3d/Slic3r) 
application's arrangement feature. It uses local optimization techniques to find
the best placement of each new item based on some features of the arrangement.

In the near future I would like to use machine learning to evaluate the 
placements and (or) the order if items in which they are placed and see what 
results can be obtained. This is a different approach than that of SVGnest which 
uses genetic algorithms to find better and better selection orders. Maybe the 
two approaches can be combined as well.

# Integration 

Integrating the library can be done in two ways. Use whichever suits best.

1. Copying source files directly into a target project: The library is header 
only and its enough to just copy the content of the ```include``` directory or 
specify the location of these headers to the compiler. The project source tree can also be used as a subdirectory (or git submodule) in any other C++ project by using ```add_subdirectory()``` command in the parent level CMakeLists.txt file.

1. Compile a shared or dynamic library with specific geometry and optimization backend and link to it. To do this, disable the ```LIBNEST2D_HEADER_ONLY``` option in the CMake configuration. Currently, the only available geometry backend is [Clipper](http://www.angusj.com/delphi/clipper.php) and the optimization features are provided by the [NLopt](https://nlopt.readthedocs.io/en/latest/) library. These can be selected
with the ```LIBNEST2D_GEOMETRIES``` and ```LIBNEST2D_OPTIMIZER``` cache variables
in CMake.

# Example

A simple example may be the best way to demonstrate the usage of the library.

``` c++
#include <iostream>
#include <string>

// Here we include the libnest2d library
#include <libnest2d.h>

int main(int argc, const char* argv[]) {
    using namespace libnest2d;

    // Example polygons 
    std::vector<Item> input1(23,
    {
        {-5000000, 8954050},
        {5000000, 8954050},
        {5000000, -45949},
        {4972609, -568550},
        {3500000, -8954050},
        {-3500000, -8954050},
        {-4972609, -568550},
        {-5000000, -45949},
        {-5000000, 8954050},
    });
    std::vector<Item> input2(15,
    {
       {-11750000, 13057900},
       {-9807860, 15000000},
       {4392139, 24000000},
       {11750000, 24000000},
       {11750000, -24000000},
       {4392139, -24000000},
       {-9807860, -15000000},
       {-11750000, -13057900},
       {-11750000, 13057900},
    });

    std::vector<Item> input;
    input.insert(input.end(), input1.begin(), input1.end());
    input.insert(input.end(), input2.begin(), input2.end());

    // Perform the nesting with a box shaped bin
    auto result = nest(input, Box(150000000, 150000000));

    // Retrieve resuling geometries
    for(auto& r : result) {
        for(Item& item : r) {
            auto polygon = item.transformedShape();
            // render polygon...
        }
    }

    return EXIT_SUCCESS;
}
```

It is worth to note that the type of the polygon carried by the Item objects is
the type defined as a polygon by the geometry backend. In the example we use the
clipper backend and clipper works with integer coordinates.

Of course it is possible to configure the nesting in every possible way. The
```nest``` function can take placer and selection algorithms as template parameters and their configuration as runtime parameter. It is also possible to pass a progress indication functor and a stop condition predicate to control the nesting process. For more details see the ```libnest2d.h``` header file.

## Example output

![Alt text](examples/example.svg)


# References
- [SVGNest](https://github.com/Jack000/SVGnest)
- [An effective heuristic for the two-dimensional irregular
bin packing problem](http://www.cs.stir.ac.uk/~goc/papers/EffectiveHueristic2DAOR2013.pdf)
- [Complete and robust no-fit polygon generation for the irregular stock cutting problem](https://www.sciencedirect.com/science/article/abs/pii/S0377221706001639)
- [Applying Meta-Heuristic Algorithms to the Nesting
Problem Utilising the No Fit Polygon](http://www.graham-kendall.com/papers/k2001.pdf)
- [A comprehensive and robust procedure for obtaining the nofit polygon
using Minkowski sums](https://www.sciencedirect.com/science/article/pii/S0305054806000669)