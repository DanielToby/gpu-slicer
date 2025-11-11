#include "spatial_index/no_spatial_index.hpp"

namespace slicer {

namespace {

[[nodiscard]] bool allPointsBelow(const Triangle3D& triangle, const Span& zSpan) {
    return triangle.v0.z < zSpan.lower() && triangle.v1.z < zSpan.lower() && triangle.v2.z < zSpan.lower();
}

[[nodiscard]] bool allPointsAbove(const Triangle3D& triangle, const Span& zSpan) {
    return triangle.v0.z > zSpan.upper() && triangle.v1.z > zSpan.upper() && triangle.v2.z > zSpan.upper();
}

[[nodiscard]] bool intersects(const Triangle3D& triangle, const Span& zSpan) {
    return !allPointsBelow(triangle, zSpan) && !allPointsAbove(triangle, zSpan);
}

}

void NoSpatialIndex::build(const std::vector<Triangle3D>& triangles) {
    m_triangles = triangles;
}

std::vector<Triangle3D> NoSpatialIndex::query(const Span& inRange) const {
    std::vector<Triangle3D> result;
    for (const auto& triangle : m_triangles) {
        if (intersects(triangle, inRange)) {
            result.push_back(triangle);
        }
    }
    return result;
}

BBox3D NoSpatialIndex::AABB() const {
    BBox3D result;
    for (const auto& triangle : m_triangles) {
        result.extend(getAABB(triangle));
    }
    return result;
}

}