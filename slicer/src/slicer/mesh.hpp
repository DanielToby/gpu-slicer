#pragma once

#include <vector>

#include "slicer/bbox.hpp"

namespace slicer {

struct Mesh {
    std::vector<Triangle3D> triangles;
};

//! Returns the axis-aligned bbox of the mesh. Throws if mesh is empty!
[[nodiscard]] BBox3D getAABB(const Mesh& mesh);

}