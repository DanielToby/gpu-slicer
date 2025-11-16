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
    const BBox2D& bbox,
    const std::vector<Polygon2D>& polygons,
    const std::string& path,
    float scaleFactor) {
    auto scaledBBox = bbox * scaleFactor;

    const double width = scaledBBox.max.x - scaledBBox.min.x;
    const double height = scaledBBox.max.y - scaledBBox.min.y;
    std::ofstream out(path);
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" "
        << "width=\"" << width << "\" height=\"" << height << "\" "
        << "viewBox=\"0 0 " << width << " " << height << "\">\n";

    out << "<g stroke=\"black\" stroke-width=\"1\" fill=\"none\">\n";

    auto tx = [&](float x) {
        return (x * scaleFactor) - scaledBBox.min.x;
    };

    // Flip Y so positive Y is up in the SVG
    auto ty = [&](float y) {
        return scaledBBox.max.y - (y * scaleFactor);
    };

    for (const auto& polygon : polygons) {
        out << toSVG(polygon, tx, ty);
    }

    out << "</g>\n</svg>\n";
}

}