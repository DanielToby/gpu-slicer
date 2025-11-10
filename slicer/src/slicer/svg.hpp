#pragma once

#include <vector>
#include <string>

#include "slicer/bbox.hpp"

namespace slicer {

void writeSVG(
    const BBox2D& dimensions,
    const std::vector<Triangle2D>& triangles,
    const std::string& path);

}