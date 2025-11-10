#pragma once

#include <vector>

#include "slicer/mesh.hpp"

namespace slicer {

struct Slice {
    std::vector<Triangle2D> triangles;
};

//! The first height will be the lowest point in the mesh. Each subsequent height is thickness_mm above the last.
[[nodiscard]] std::vector<double> getSliceHeights(const BBox3D& volume, float thickness_mm);

}