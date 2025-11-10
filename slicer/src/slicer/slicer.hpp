#pragma once

#include <vector>

#include "slicer/mesh.hpp"

namespace slicer {

struct SliceDimensions {
    float z0;
    float z1;

    [[nodiscard]] bool operator==(const SliceDimensions& other) const {
        return z0 == other.z0 && z1 == other.z1;
    }
};

struct Slice {
    std::vector<Triangle2D> triangles;
    SliceDimensions dimensions;
};

//! The first height will be the lowest point in the mesh. Each subsequent height is thickness_mm above the last.
[[nodiscard]] std::vector<SliceDimensions> getSliceHeights(const BBox3D& volume, float thickness_mm);

[[nodiscard]] std::vector<Slice> slice(const Mesh& volume, float thickness_mm);

}
