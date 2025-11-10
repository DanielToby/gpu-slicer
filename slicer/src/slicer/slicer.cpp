#include "slicer/slicer.hpp"

#include <ranges>

namespace slicer {

namespace {

[[nodiscard]] bool intersects(const Vec3& point, float zMin, float zMax) {
    return point.z >= zMin && point.z <= zMax;
}

[[nodiscard]] bool intersects(const Triangle3D& triangle, float zMin, float zMax) {
    return intersects(triangle.v0, zMin, zMax) || intersects(triangle.v1, zMin, zMax) || intersects(triangle.v2, zMin, zMax);
}

[[nodiscard]] std::vector<Triangle3D> getTrianglesInRange(const Mesh& mesh, float zMin, float zMax) {
    std::vector<Triangle3D> result;
    for (const auto& triangle : mesh.triangles) {
        if (intersects(triangle, zMin, zMax)) {
            result.push_back(triangle);
        }
    }
    return result;
}

[[nodiscard]] Vec2 toVec2(const Vec3& vec3) {
    return {vec3.x, vec3.y};
}

[[nodiscard]] Triangle2D toTriangle2D(const Triangle3D& triangle) {
    return {toVec2(triangle.v0), toVec2(triangle.v1), toVec2(triangle.v2)};
}

}

std::vector<SliceDimensions> getSliceHeights(const BBox3D& volume, float thickness_mm) {
    std::vector<SliceDimensions> result;
    const auto bottom = volume.min.z;

    auto current = bottom;
    do {
        auto next = current + thickness_mm;
        result.push_back({current, std::min(next, volume.max.z)});
        current = next;
    } while (current < volume.max.z);

    return result;
}

std::vector<Slice> slice(const Mesh& mesh, float thickness_mm) {
    std::vector<Slice> result;

    auto sliceHeights = getSliceHeights(getAABB(mesh), thickness_mm);
    for (const auto& sliceHeight : sliceHeights) {
        // This is the query we can optimize later with a spatial data structure.
        auto triangles =
            mesh.triangles | std::views::filter([&sliceHeight](const auto& triangle) {
                return intersects(triangle, sliceHeight.z0, sliceHeight.z1);
            }) |
            std::views::transform([](const auto& triangle3D) {
                return toTriangle2D(triangle3D);
            });

        result.push_back(Slice{{triangles.begin(), triangles.end()}, sliceHeight});
    }

    return result;
}

}
