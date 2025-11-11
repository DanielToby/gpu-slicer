#include "slicer/clipper.hpp"

namespace slicer {

namespace {

//! Starting at or crossing the zPosition from outside the keepRegion is considered entering.
[[nodiscard]] bool entersRegion(const Vec3& p0, const Vec3& p1, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return (p0.z <= zPosition && zPosition < p1.z);
    case KeepRegion::Below:
        return (p0.z >= zPosition && zPosition > p1.z);
    }
    throw std::invalid_argument("Invalid keep region");
}

//! Crossing or arriving at the zPosition from inside the keepRegion is considered exiting.
[[nodiscard]] bool exitsRegion(const Vec3& p0, const Vec3& p1, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return (p0.z > zPosition && zPosition >= p1.z);
    case KeepRegion::Below:
        return (p0.z < zPosition && zPosition <= p1.z);
    }
    throw std::invalid_argument("Invalid keep region");
}

[[nodiscard]] bool inRegion(const Vec3& p0, const Vec3& p1, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return (p0.z > zPosition && p1.z > zPosition);
    case KeepRegion::Below:
        return (p0.z < zPosition && p1.z < zPosition);
    }
    throw std::invalid_argument("Invalid keep region");
}

[[nodiscard]] std::optional<std::size_t> getFirstIndexWhere(const std::vector<Vec3>& vertices, const std::function<bool(const Vec3&)>& predicate) {
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        if (std::invoke(predicate, vertices[i])) {
            return i;
        }
    }
    return std::nullopt;
}

//! This checks for strictly above / below but NOT equal to ZPosition, so we can guarantee that there is complete geometry in keepRegion.
[[nodiscard]] std::optional<std::size_t> getStartingIndex(const std::vector<Vec3>& vertices, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
        case KeepRegion::Above:
            return getFirstIndexWhere(vertices, [&zPosition](const Vec3& vertex) { return vertex.z > zPosition; });
        case KeepRegion::Below:
            return getFirstIndexWhere(vertices, [&zPosition](const Vec3& vertex) { return vertex.z < zPosition; });
    }
    throw std::invalid_argument("Invalid keep region");
}

[[nodiscard]] float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodsicard]] bool allPointsOnZPosition(const std::vector<Vec3>& vertices, float zPosition) {
    return std::ranges::all_of(vertices, [&zPosition](const Vec3& v) {
        return v.z == zPosition;
    });
}

}

Vec3 detail::intersect(Vec3 p0, Vec3 p1, float zPosition) {
    // X(t) = L0 + t * D, where L0 is P0, and D (direction) is p1 - p0.
    const auto line = Line3D::fromPoints(p0, p1);

    // For any point X: dot((P0 - X), N) = 0, where P0 is zPosition at the origin, and N (normal) is the Z-axis.
    const auto plane = Plane{{0, 0, zPosition}, {0, 0, 1}};

    // Substituting line equation into plane and solving: t = dot((P0 - L0), N) / dot(D, N).
    const auto t = dot((plane.p0 - line.p0), plane.normal) / dot(line.direction, plane.normal);
    return line.p0 + line.direction * t;
}

Polygon3D clip(const Polygon3D& polygon, float zPosition, KeepRegion keepRegion) {
    if (!polygon.isValid()) {
        throw std::invalid_argument("Invalid polygon.");
    }

    // Special case: All points on zPosition:
    if (allPointsOnZPosition(polygon.vertices, zPosition) && keepRegion == KeepRegion::Above) {
        return polygon;
    }

    auto result = Polygon3D{};
    auto offset = getStartingIndex(polygon.vertices, zPosition, keepRegion);
    if (!offset) {
        // No geometry in region.
        return {};
    }

    for (auto i = 0; i < polygon.vertices.size(); ++i) {
        const auto p0 = polygon.vertices[(i + *offset) % polygon.vertices.size()];
        const auto p1 = polygon.vertices[(i + *offset + 1) % polygon.vertices.size()];

        if (entersRegion(p0, p1, zPosition, keepRegion)) {
            result.vertices.push_back(detail::intersect(p0, p1, zPosition));
        } else if (exitsRegion(p0, p1, zPosition, keepRegion)) {
            result.vertices.push_back(p0);
            result.vertices.push_back(detail::intersect(p0, p1, zPosition));
        } else if (inRegion(p0, p1, zPosition, keepRegion)) {
            result.vertices.push_back(p0);
        }
    }

    return result;
}

}