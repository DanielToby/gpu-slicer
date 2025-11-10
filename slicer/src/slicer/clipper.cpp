#include "slicer/clipper.hpp"

#include "mesh.hpp"

namespace slicer {

namespace {

[[nodiscard]] std::size_t getFirstIndexAboveYPosition(const std::vector<Vec3>& vertices, float zPosition) {
    if (vertices.empty()) {
        throw std::invalid_argument("Empty vertices.");
    }
    for (std::size_t i = 0; i < vertices.size(); ++i) {
        if (vertices[i].z >= zPosition) {
            return i;
        }
    }
    throw std::runtime_error("No vertex above zPosition.");
}

[[nodiscard]] float dot(const Vec3& a, const Vec3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

[[nodiscard]] Polygon3D clipAbove(const Polygon3D& polygon, float zPosition) {
    if (!polygon.isValid()) {
        throw std::invalid_argument("Invalid polygon.");
    }

    auto bbox = getAABB(polygon);
    if (bbox.min.z >= zPosition) {
        // Entire polygon lies above clipping plane.
        return polygon;
    }
    if (bbox.max.z <= zPosition) {
        // Entire polygon lies below clipping plane.
        return Polygon3D{};
    }

    auto result = Polygon3D{};

    // We start at the first vertex above yPosition.
    auto offset = getFirstIndexAboveYPosition(polygon.vertices, zPosition);
    result.vertices.push_back(polygon.vertices.at(offset));

    bool isAbove = true;
    for (auto i = 0; i < polygon.vertices.size(); ++i) {
        auto i0 = (i + offset) % polygon.vertices.size();
        auto i1 = (i + 1 + offset) % polygon.vertices.size();

        auto p0 = polygon.vertices.at(i0);
        auto p1 = polygon.vertices.at(i1);

        if (p1.z >= zPosition) {
            if (isAbove) {
                result.vertices.push_back(p1);
            } else {
                // Crossing yPosition! Find intersection:
                if (auto intersection = detail::intersect(p0, p1, zPosition)) {
                    result.vertices.push_back(*intersection);
                } else {
                    throw std::runtime_error("Invalid intersection.");
                }

                // Is above should only become true one time. This vertex was the initial vertex.
                if (p1 != result.vertices.at(0)) {
                    throw std::runtime_error("Polygon is not convex.");
                }
            }
        } else {
            if (isAbove) {
                //! Crossing yPosition! Find intersection:
                if (auto intersection = detail::intersect(p0, p1, zPosition)) {
                    result.vertices.push_back(*intersection);
                } else {
                    throw std::runtime_error("Invalid intersection.");
                }
            }
            isAbove = false;
        }
    }

    return result;
}

}

std::optional<Vec3> detail::intersect(Vec3 p0, Vec3 p1, float zPosition) {
    // Special case: line along zPosition:
    if (p0.z == zPosition && p1.z == zPosition) {
        return p0;
    }

    if (!(p0.z <= zPosition && p1.z > zPosition) && !(p1.z <= zPosition && p0.z > zPosition)) {
        return std::nullopt;
    }

    // X(t) = L0 + t * D, where L0 is P0, and D (direction) is p1 - p0.
    const auto line = Line3D::fromPoints(p0, p1);

    // For any point X: dot((P0 - X), N) = 0, where P0 is zPosition at the origin, and N (normal) is the Z-axis.
    const auto plane = Plane{{0, 0, zPosition}, {0, 0, 1}};

    // Substituting line equation into plane and solving: t = dot((P0 - L0), N) / dot(D, N).
    const auto t = dot((plane.p0 - line.p0), plane.normal) / dot(line.direction, plane.normal);
    return line.p0 + line.direction * t;
}

Polygon3D clip(const Polygon3D& polygon, float zPosition, KeepRegion keepRegion) {
    switch (keepRegion) {
    case KeepRegion::Above:
        return clipAbove(polygon, zPosition);
    default:
        throw std::invalid_argument("Not implemented.");
    }
}

}