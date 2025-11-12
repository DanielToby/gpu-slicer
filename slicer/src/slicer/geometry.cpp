#include "slicer/geometry.hpp"

namespace slicer {

namespace {

template <typename T>
struct Line {
    T p0;
    T direction;

    [[nodiscard]] static Line fromPoints(const T& p0, const T& p1) {
        return {p0, p1 - p0};
    }
};

using Line3D = Line<Vec3>;

struct Plane {
    Vec3 p0;
    Vec3 normal;
};

[[nodiscard]] float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

}

Vec3 intersect(const Vec3& p0, const Vec3& p1, float zPosition) {
    // X(t) = L0 + t * D, where L0 is P0, and D (direction) is p1 - p0.
    const auto line = Line3D::fromPoints(p0, p1);

    // For any point X: dot((P0 - X), N) = 0, where P0 is zPosition at the origin, and N (normal) is the Z-axis.
    const auto plane = Plane{{0, 0, zPosition}, {0, 0, 1}};

    // Substituting line equation into plane and solving: t = dot((P0 - L0), N) / dot(D, N).
    const auto t = dot((plane.p0 - line.p0), plane.normal) / dot(line.direction, plane.normal);
    return line.p0 + line.direction * t;
}

}