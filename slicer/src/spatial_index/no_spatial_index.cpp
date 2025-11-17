#include "spatial_index/no_spatial_index.hpp"

namespace slicer {

namespace {

[[nodiscard]] bool allPointsBelow(const Triangle3D& triangle, float zPosition) {
    return triangle.v0.z() < zPosition && triangle.v1.z() < zPosition && triangle.v2.z() < zPosition;
}

[[nodiscard]] bool allPointsAbove(const Triangle3D& triangle, float zPosition) {
    return triangle.v0.z() > zPosition && triangle.v1.z() > zPosition && triangle.v2.z() > zPosition;
}

[[nodiscard]] bool anyPointOnZ(const Triangle3D& triangle, float zPosition) {
    return triangle.v0.z() == zPosition || triangle.v1.z() == zPosition || triangle.v2.z() == zPosition;
}

[[nodiscard]] bool intersects(const Triangle3D& triangle, float zPosition) {
    return anyPointOnZ(triangle, zPosition) || (!allPointsBelow(triangle, zPosition) && !allPointsAbove(triangle, zPosition));
}

}

void NoSpatialIndex::build(const std::vector<Triangle3D>& triangles) {
    m_triangles = triangles;
}

std::vector<Triangle3D> NoSpatialIndex::query(float zPosition) const {
    std::vector<Triangle3D> result;
    for (const auto& triangle : m_triangles) {
        if (intersects(triangle, zPosition)) {
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