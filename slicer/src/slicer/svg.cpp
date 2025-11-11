#include "slicer/svg.hpp"

#include <fstream>

namespace slicer {

namespace {

[[nodiscard]] std::string toSVG(const Polygon2D& polygon, std::function<float(float)> tx, std::function<float(float)> ty) {
    // TODO: fmtlib.
    auto result = std::string{"<polygon points=\""};
    for (const auto& v : polygon.vertices) {
        result += (std::to_string(tx(v.x)) + ", " + std::to_string(ty(v.y)) + " ");
    }
    result += "\" />\n";
    return result;
}

}

void writeSVG(
    const BBox2D& dimensions,
    const std::vector<Polygon2D>& polygons,
    const std::string& path) {
    const double width = dimensions.max.x - dimensions.min.x;
    const double height = dimensions.max.y - dimensions.min.y;
    std::ofstream out(path);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << width << "\" height=\"" << height << "\" "
        << "viewBox=\"0 0 " << width << " " << height << "\">\n";

    out << "<g stroke=\"black\" stroke-width=\"1\" fill=\"none\">\n";

    auto tx = [&dimensions](float x) {
        return x - dimensions.min.x;
    };

    // Flip Y so positive Y is up in the SVG
    auto ty = [&dimensions](float y) {
        return (dimensions.max.y - y);
    };

    for (const auto& polygon : polygons) {
        out << toSVG(polygon, tx, ty);
    }

    out << "</g>\n</svg>\n";
}

}