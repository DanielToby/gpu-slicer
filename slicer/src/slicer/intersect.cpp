#include "slicer/intersect.hpp"

namespace slicer {

namespace {

//! This makes describing intersection edge cases easier .
struct ClassifiedTriangle {
    std::array<QuantizedPoint3D, 3> points;

    std::array<std::size_t, 3> below{};
    std::array<std::size_t, 3> on{};
    std::array<std::size_t, 3> above{};

    std::size_t belowCount = 0;
    std::size_t onCount = 0;
    std::size_t aboveCount = 0;

    [[nodiscard]] QuantizedPoint3D getPointBelowZ(std::size_t i) const { return points[below[i]]; }
    [[nodiscard]] QuantizedPoint3D getPointOnZ(std::size_t i) const { return points[on[i]]; }
    [[nodiscard]] QuantizedPoint3D getPointAboveZ(std::size_t i) const { return points[above[i]]; }
};

ClassifiedTriangle classify(const Triangle3D& triangle, float z) {
    ClassifiedTriangle result{
        {QuantizedPoint3D::fromPoint(triangle.v0), QuantizedPoint3D::fromPoint(triangle.v1), QuantizedPoint3D::fromPoint(triangle.v2)},
    };
    for (std::size_t i = 0; i < 3; ++i) {
        const float pz = result.points[i].value().z();

        if (pz < z) {
            result.below[result.belowCount++] = i;
        } else if (pz > z) {
            result.above[result.aboveCount++] = i;
        } else {
            result.on[result.onCount++] = i;
        }
    }

    return result;
}

[[nodiscard]] QuantizedPoint3D getIntersectionOrThrow(const QuantizedPoint3D& lower, const QuantizedPoint3D& upper, float zPosition) {
    auto result = intersect(QuantizedLine3D{lower, upper}, zPosition);
    if (!result) {
        throw std::runtime_error("Bad call to intersect.");
    }
    return *result;
}

[[nodiscard]] float getDeterminant(const Vec2& p0, const Vec2& p1) {
    return p0.x() * p1.y() - p1.x() * p0.y();
}

}

bool intersects(const Segment3D& edge, float zPosition)  {
    return (edge.v0.z() < zPosition && edge.v1.z() > zPosition)
    || (edge.v0.z() > zPosition && edge.v1.z() < zPosition);
}

std::optional<QuantizedPoint3D> intersect(const QuantizedLine3D& line, float zPosition) {
    // X(t) = L0 + t * D, where L0 is P0, and D (direction) is p1 - p0.
    const auto ray = Ray3D::fromPoints(line.v0.value(), line.v1.value());

    // For any point X: dot((P0 - X), N) = 0, where P0 is zPosition at the origin, and N (normal) is the Z-axis.
    const auto plane = Plane{{0, 0, zPosition}, {0, 0, 1}};

    // Substituting line equation into plane and solving: t = dot((P0 - L0), N) / dot(D, N).
    const auto d = Vec3::dot(ray.direction, plane.normal);
    if (d == 0) {
        return std::nullopt;
    }

    const auto t = Vec3::dot((plane.p0 - ray.p0), plane.normal) / d;
    if (0 <= t && t <= 1) {
        return QuantizedPoint3D::fromPoint(Vec3{ray.p0 + ray.direction * t});
    }

    return std::nullopt;
}

std::optional<Vec2> intersect(const Segment2D& line, const Ray2D& ray) {
    const auto s = line.v1 - line.v0;
    const auto rxs = getDeterminant(ray.direction, s);
    if (rxs == 0) {
        return std::nullopt;
    }

    const auto c = line.v0 - ray.p0;
    const auto t = getDeterminant(c, s) / rxs;
    const auto u = getDeterminant(c, ray.direction) / rxs;

    if (0 <= t && 0 <= u && u <= 1) {
        return {line.v0 + (line.v1 - line.v0) * u};
    }
    return std::nullopt;
}

Vec2 quantize(const Vec2& in) {
    return {
        std::round(in.x() / EPSILON) * EPSILON,
        std::round(in.y() / EPSILON) * EPSILON};
}

Vec3 quantize(const Vec3& in) {
    return {
        std::round(in.x() / EPSILON) * EPSILON,
        std::round(in.y() / EPSILON) * EPSILON,
        std::round(in.z() / EPSILON) * EPSILON};
}

std::size_t QuantizedPoint2DHash::operator()(const QuantizedPoint2D& v) const noexcept {
    return Vec2Hash{}(v.value());
}

std::size_t QuantizedLine2DHash::operator()(const QuantizedLine2D& v) const noexcept {
    return -1;
}

std::optional<QuantizedLine2D> intersect(const Triangle3D& triangle, float zPosition) {
    auto classified = classify(triangle, zPosition);

    // Empty cases:
    // If all three are on, nothing is returned.
    if (classified.onCount == 3) {
        return std::nullopt;
    }
    // If two are on and one is below, nothing is returned.
    if (classified.onCount == 2 && classified.belowCount == 1) {
        return std::nullopt;
    }
    // If one is on and two are above or below, nothing is returned.
    if (classified.onCount == 1 && (classified.aboveCount == 2 || classified.belowCount == 2)) {
        return std::nullopt;
    }

    const auto makeLine = [](const QuantizedPoint3D& a, const QuantizedPoint3D& b) {
        return QuantizedLine2D{a.as<Vec2>(), b.as<Vec2>()};
    };

    // Valid cases:
    // If two are on and one is above, the line between the two is returned.
    if (classified.onCount == 2 && classified.aboveCount == 1) {
        return makeLine(classified.getPointOnZ(0), classified.getPointOnZ(1));
    }
    // If one is on and one is above and one is below, the line between the on-one and an intersection is returned.
    if (classified.onCount == 1 && classified.belowCount == 1 && classified.aboveCount == 1) {
        const auto intersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        return makeLine(classified.getPointOnZ(0), intersection);
    }
    // If one is above and two are below, the two intersections are returned.
    if (classified.aboveCount == 1 && classified.belowCount == 2) {
        const auto firstIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        const auto secondIntersection = getIntersectionOrThrow(classified.getPointBelowZ(1), classified.getPointAboveZ(0), zPosition);
        return makeLine(firstIntersection, secondIntersection);
    }
    // If one is below and two are above, the two intersections are returned.
    if (classified.belowCount == 1 && classified.aboveCount == 2) {
        const auto firstIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        const auto secondIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(1), zPosition);
        return makeLine(firstIntersection, secondIntersection);
    }

    throw std::runtime_error("Unhandled triangle intersection case.");
}

IntersectData intersect(std::span<const Triangle3D> triangles, float zPosition) {
    IntersectData result;
    for (const auto& triangle : triangles) {
        if (auto intersection = intersect(triangle, zPosition)) {
            result.vertices.insert(intersection->v0);
            result.vertices.insert(intersection->v1);
            result.edges.insert(*intersection);
        }
    }

    return result;
}

}