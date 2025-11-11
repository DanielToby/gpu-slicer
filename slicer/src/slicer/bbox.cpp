#include "slicer/bbox.hpp"

namespace slicer {

BBox2D getAABB(const Polygon2D& polygon) {
    auto result = BBox2D{};
    for (const auto& vertex : polygon.vertices) {
        result.extend(vertex);
    }
    return result;
}

BBox3D getAABB(const Polygon3D& polygon) {
    auto result = BBox3D{};
    for (const auto& vertex : polygon.vertices) {
        result.extend(vertex);
    }
    return result;
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