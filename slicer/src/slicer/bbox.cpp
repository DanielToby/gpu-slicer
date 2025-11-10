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

}