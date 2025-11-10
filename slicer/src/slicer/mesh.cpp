#include "slicer/mesh.hpp"

namespace slicer {

namespace {

//! Returns the combined minimum of a and b.
[[nodiscard]] Vec3 min(const Vec3& a, const Vec3& b) {
    return {std::min(a.x, b.x), std::min(a.y, b.y), std::min(a.z, b.z)};
}

//! Returns the combined maximum of a and b.
[[nodiscard]] Vec3 max(const Vec3& a, const Vec3& b) {
    return {std::max(a.x, b.x), std::max(a.y, b.y), std::max(a.z, b.z)};
}

//! Returns the combined minimum of a and b.
[[nodiscard]] Vec3 min(const Vec3& a, const Triangle3D& b) {
    auto result = a;
    result = min(result, b.v0);
    result = min(result, b.v1);
    result = min(result, b.v2);
    return result;
}

//! Returns the combined minimum of a and b.
[[nodiscard]] Vec3 max(const Vec3& a, const Triangle3D& b) {
    auto result = a;
    result = max(result, b.v0);
    result = max(result, b.v1);
    result = max(result, b.v2);
    return result;
}

}

BBox3D getAABB(const Mesh& mesh) {
    if (mesh.triangles.empty()) {
        throw std::runtime_error("Empty mesh.");
    }

    BBox3D result;
    for (const auto& triangle : mesh.triangles) {
        result.min = min(result.min, triangle);
        result.max = max(result.max, triangle);
    }
    return result;
}

}