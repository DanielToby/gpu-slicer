#include "slicer/stl.hpp"

#include <stl_reader.h>

#include <iostream>

namespace {

[[nodiscard]] slicer::Vec3 getNormal(const stl_reader::StlMesh<float, unsigned int>& mesh, std::size_t triangleIndex) {
    const float* n = mesh.tri_normal(triangleIndex);
    return slicer::Vec3{n[0], n[1], n[2]};
}

[[nodiscard]] slicer::Vec3 getCorner(const stl_reader::StlMesh<float, unsigned int>& mesh, std::size_t triangleIndex, std::size_t cornerIndex) {
    const float* c = mesh.tri_corner_coords(triangleIndex, cornerIndex);
    return {c[0], c[1], c[2]};
}

}

namespace slicer {

std::vector<Triangle3D> loadStl(const std::string& fileName) noexcept(false) {
    std::vector<Triangle3D> result;

    const auto mesh = stl_reader::StlMesh(fileName);
    for (auto triangleIndex = std::size_t{0}; triangleIndex < mesh.num_tris(); ++triangleIndex) {
        result.push_back(Triangle3D{
            getNormal(mesh, triangleIndex),
            getCorner(mesh, triangleIndex, 0),
            getCorner(mesh, triangleIndex, 1),
            getCorner(mesh, triangleIndex, 2),
        });
    }

    return result;
}

}