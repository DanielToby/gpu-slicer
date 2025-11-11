#pragma once

#include <vector>
#include <string>

#include "slicer/bbox.hpp"

namespace slicer {

void writeSVG(
    const BBox2D& dimensions,
    const std::vector<Polygon2D>& polygons,
    const std::string& path);

}