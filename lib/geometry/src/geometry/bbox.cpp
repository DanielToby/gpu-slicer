#include "geometry/bbox.hpp"

namespace slicer {

BBox2D getAABB(std::span<const Vec2> vertices) {
    auto result = BBox2D{};
    for (const auto& vertex : vertices) {
        result.extend(vertex);
    }
    return result;
}

BBox3D getAABB(std::span<const Vec3> vertices) {
    auto result = BBox3D{};
    for (const auto& vertex : vertices) {
        result.extend(vertex);
    }
    return result;
}

BBox2D getAABB(const Polygon2D& polygon) {
    return getAABB(polygon.vertices);
}

BBox3D getAABB(const Polygon3D& polygon) {
    return getAABB(polygon.vertices);
}

BBox2D getAABB(const Triangle2D& triangle) {
    BBox2D result;
    result.extend(triangle.v0);
    result.extend(triangle.v1);
    result.extend(triangle.v2);
    return result;
}

BBox3D getAABB(const Triangle3D& triangle) {
    BBox3D result;
    result.extend(triangle.v0);
    result.extend(triangle.v1);
    result.extend(triangle.v2);
    return result;
}

}