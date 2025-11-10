#include "slicer/svg.hpp"

#include <fstream>

namespace slicer {

void writeSVG(
    const BBox2D& dimensions,
    const std::vector<Triangle2D>& triangles,
    const std::string& path) {
    const double min_x = dimensions.min.x;
    const double min_y = dimensions.min.y;
    const double max_y = dimensions.max.y;

    const double width = dimensions.max.x - dimensions.min.x;
    const double height = dimensions.max.y - dimensions.min.y;

    auto tx = [&](double x) {
        return x - min_x;
    };

    // Flip Y so positive Y is up in the SVG
    auto ty = [&](double y) {
        return (max_y - y);
    };

    std::ofstream out(path);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << width << "\" height=\"" << height << "\" "
        << "viewBox=\"0 0 " << width << " " << height << "\">\n";

    out << "<g stroke=\"black\" stroke-width=\"1\" fill=\"none\">\n";

    for (auto& t : triangles) {
        out << "<polygon points=\""
            << tx(t.v0.x) << "," << ty(t.v0.y) << " "
            << tx(t.v1.x) << "," << ty(t.v1.y) << " "
            << tx(t.v2.x) << "," << ty(t.v2.y)
            << "\" />\n";
    }

    out << "</g>\n</svg>\n";
}

}