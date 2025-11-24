#pragma once

#include <vector>
#include <string>

#include "geometry/bbox.hpp"

namespace slicer {

void writeSVG(
    const BBox2D& bbox,
    const std::vector<Polygon2D>& polygons,
    const std::string& path,
    float scaleFactor);

}