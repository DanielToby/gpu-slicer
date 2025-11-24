#pragma once

#include <vector>

#include "spatial_index/spatial_index.hpp"
#include "slicer/slice_outlines.hpp"

namespace slicer {

//! The result of slicing.
struct Slice {
    std::vector<Polygon2D> polygons;
    float zPosition;
};

//! The first height will be the lowest point in the mesh. Each subsequent height is thickness_mm above the last.
[[nodiscard]] std::vector<float> getSliceHeights(const BBox3D& volume, float thickness);

//! Slices the mesh. I might split this out more later to demonstrate the value of the spatial index, since the polygon
//! construction occupies most of the slice time right now.
[[nodiscard]] std::vector<Slice> slice(const I_SpatialIndex& mesh, float thickness);

}
