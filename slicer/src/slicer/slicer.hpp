#pragma once

#include <vector>

#include "spatial_index/spatial_index.hpp"

namespace slicer {

struct Slice {
    std::vector<Polygon2D> polygons;
    Span zDimensions;
};

//! The first height will be the lowest point in the mesh. Each subsequent height is thickness_mm above the last.
[[nodiscard]] std::vector<Span> getSliceHeights(const BBox3D& volume, float thickness_mm);

[[nodiscard]] std::vector<Slice> slice(const I_SpatialIndex& mesh, float thickness_mm);

}
