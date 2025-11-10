#include "slicer/mesh.hpp"

namespace slicer {

namespace {

void extend(BBox3D& bbox, const Triangle3D& triangle) {
    bbox.extend(triangle.v0);
    bbox.extend(triangle.v1);
    bbox.extend(triangle.v2);
}

}

BBox3D getAABB(const Mesh& mesh) {
    if (mesh.triangles.empty()) {
        throw std::runtime_error("Empty mesh.");
    }

    BBox3D result;
    for (const auto& triangle : mesh.triangles) {
        extend(result, triangle);
    }
    return result;
}

}