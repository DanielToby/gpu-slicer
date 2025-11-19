#include "slicer/intersect.hpp"

#include <ostream>

namespace slicer {

namespace {

constexpr float EPSILON = 1e-4f;

[[nodiscard]] std::int64_t quantize(float v) {
    return llround(v / EPSILON);
}

[[nodiscard]] QuantizedVec2 quantize(const Vec2& v) {
    return {quantize(v.x()), quantize(v.y())};
}

[[nodiscard]] float dequantize(std::int64_t v) {
    return v * EPSILON;
}

[[nodiscard]] Vec2 dequantize(const QuantizedVec2& v) {
    return {dequantize(v.qx), dequantize(v.qy)};
}

//! This makes describing intersection edge cases easier .
struct ClassifiedTriangle {
    std::array<Vec3, 3> points;

    std::array<std::size_t, 3> below{};
    std::array<std::size_t, 3> on{};
    std::array<std::size_t, 3> above{};

    std::size_t belowCount = 0;
    std::size_t onCount = 0;
    std::size_t aboveCount = 0;

    [[nodiscard]] Vec3 getPointBelowZ(std::size_t i) const {
        return points[below[i]];
    }
    [[nodiscard]] Vec3 getPointOnZ(std::size_t i) const {
        return points[on[i]];
    }
    [[nodiscard]] Vec3 getPointAboveZ(std::size_t i) const {
        return points[above[i]];
    }
};

ClassifiedTriangle classify(const Triangle3D& triangle, float z) {
    ClassifiedTriangle result{
        {triangle.v0, triangle.v1, triangle.v2},
    };
    for (std::size_t i = 0; i < 3; ++i) {
        const float pz = result.points[i].z();

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

[[nodiscard]] Vec3 getIntersectionOrThrow(const Vec3& lower, const Vec3& upper, float zPosition) {
    auto result = intersect(Segment3D{lower, upper}, zPosition);
    if (!result) {
        throw std::runtime_error("Bad call to intersect.");
    }
    return *result;
}

}

QuantizedVec2::QuantizedVec2(float x, float y) : QuantizedVec2(quantize(x), quantize(y)) {}

Vec2 QuantizedVec2::toVec2() const noexcept { return dequantize(*this); }

std::optional<Vec3> intersect(const Segment3D& segment, float zPosition) {
    // X(t) = L0 + t * D, where L0 is P0, and D (direction) is p1 - p0.
    const auto ray = Ray3D::fromPoints(segment.v0, segment.v1);

    // For any point X: dot((P0 - X), N) = 0, where P0 is zPosition at the origin, and N (normal) is the Z-axis.
    const auto plane = Plane{{0, 0, zPosition}, {0, 0, 1}};

    // Substituting line equation into plane and solving: t = dot((P0 - L0), N) / dot(D, N).
    const auto d = Vec3::dot(ray.direction, plane.normal);
    if (d == 0) {
        return std::nullopt;
    }

    const auto t = Vec3::dot((plane.p0 - ray.p0), plane.normal) / d;
    if (0 <= t && t <= 1) {
        return {ray.p0 + ray.direction * t};
    }
    return std::nullopt;
}

std::optional<Segment3D> intersect(const Triangle3D& triangle, float zPosition) {
    auto classified = classify(triangle, zPosition);

    // Empty cases:
    // If all three are on, nothing is returned.
    if (classified.onCount == 3 || classified.belowCount == 3 || classified.aboveCount == 3) {
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

    // Valid cases:
    // If two are on and one is above, the line between the two is returned.
    if (classified.onCount == 2 && classified.aboveCount == 1) {
        return Segment3D{classified.getPointOnZ(0), classified.getPointOnZ(1)};
    }
    // If one is on and one is above and one is below, the line between the on-one and an intersection is returned.
    if (classified.onCount == 1 && classified.belowCount == 1 && classified.aboveCount == 1) {
        const auto intersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        return Segment3D{classified.getPointOnZ(0), intersection};
    }
    // If one is above and two are below, the two intersections are returned.
    if (classified.aboveCount == 1 && classified.belowCount == 2) {
        const auto firstIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        const auto secondIntersection = getIntersectionOrThrow(classified.getPointBelowZ(1), classified.getPointAboveZ(0), zPosition);
        return Segment3D{firstIntersection, secondIntersection};
    }
    // If one is below and two are above, the two intersections are returned.
    if (classified.belowCount == 1 && classified.aboveCount == 2) {
        const auto firstIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        const auto secondIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(1), zPosition);
        return Segment3D{firstIntersection, secondIntersection};
    }

    throw std::runtime_error("Unhandled triangle intersection case.");
}

std::set<QuantizedSegment2D> intersect(std::span<const Triangle3D> triangles, float zPosition) {
    std::set<QuantizedSegment2D> result;
    for (const auto& triangle : triangles) {
        if (auto intersection = intersect(triangle, zPosition)) {
            const auto qa = quantize(intersection->v0.as<Vec2>());
            const auto qb = quantize(intersection->v1.as<Vec2>());
            result.insert({qa, qb});
        }
    }

    return result;
}
std::ostream& operator<<(std::ostream& os, const QuantizedVec2& v) {
    return os << "QuantizedVec2{qx=" << v.qx << ", qy=" << v.qy << "}";
}

std::ostream& operator<<(std::ostream& os, const QuantizedSegment2D& v) {
    return os << "QuantizedSegment2D{v0=" << v.v0 << ", v1=" << v.v1 << "}";
}

}
