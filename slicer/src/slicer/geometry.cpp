#include "slicer/geometry.hpp"

namespace slicer {

namespace {

constexpr float EPSILON = 1e-6f;

[[nodiscard]] std::array<Segment3D, 3> getSegments(const Triangle3D& triangle) {
    auto lessZ = [](const auto& a, const auto& b) { return a.z < b.z; };
    auto getOrderedSegment = [&lessZ](const auto& p0, const auto p1) -> Segment3D{
        return {std::min(p0, p1, lessZ), std::max(p0, p1, lessZ)};
    };
    return {
        getOrderedSegment(triangle.v0, triangle.v1),
        getOrderedSegment(triangle.v1, triangle.v2),
        getOrderedSegment(triangle.v2, triangle.v0)
    };
}

[[nodiscard]] bool intersects(const Triangle3D& polygon, float zPosition) {
    return std::ranges::any_of(getSegments(polygon), [&zPosition](const Segment3D& segment) { return intersects(segment, zPosition); });
}

[[nodiscard]] float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

[[nodiscard]] float fractionAlong(float p0, float p1, float p) {
    if (p < p0 || p > p1 || p0 == p1) {
        throw std::invalid_argument("Invalid fraction along point.");
    }
    return (p - p0) / (p1 - p0);
}

[[nodiscard]] Vec3 intersectSegment(const Segment3D& segment, float zPosition) {
    const auto t = fractionAlong(segment.v0.z, segment.v1.z, zPosition);
    return {
        lerp(segment.v0.x, segment.v1.x, t),
        lerp(segment.v0.y, segment.v1.y, t),
        zPosition};
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

    [[nodiscard]] Vec3 getPointBelowZ(std::size_t i) { return points[below[i]]; }
    [[nodiscard]] Vec3 getPointOnZ(std::size_t i) { return points[on[i]]; }
    [[nodiscard]] Vec3 getPointAboveZ(std::size_t i) { return points[above[i]]; }
};

ClassifiedTriangle classify(const Triangle3D& triangle, float z) {
    ClassifiedTriangle result{{triangle.v0, triangle.v1, triangle.v2},};
    for (std::size_t i = 0; i < 3; ++i) {
        const float pz = result.points[i].z;

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

std::size_t Vec2Hash::operator()(const Vec2& v) const noexcept {
    // Convert to integer grid coordinates
    const auto qx = static_cast<int64_t>(std::llround(v.x / EPSILON));
    const auto qy = static_cast<int64_t>(std::llround(v.y / EPSILON));

    // Combine using a strong integer hash
    std::size_t h = std::hash<int64_t>{}(qx);
    h ^= std::hash<int64_t>{}(qy) + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);

    return h;
}

bool intersects(const Segment3D& edge, float zPosition)  {
    return (edge.v0.z < zPosition && edge.v1.z > zPosition)
    || (edge.v0.z > zPosition && edge.v1.z < zPosition);
}

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
        return ray.p0 + ray.direction * t;
    }

    return std::nullopt;
}

std::optional<Vec2> intersect(const Segment2D& segment, const Ray2D& ray) {
    const auto s = segment.v1 - segment.v0;
    const auto rxs = Vec2::cross(ray.direction, s);
    if (rxs == 0) {
        return std::nullopt;
    }

    const auto c = segment.v0 - ray.p0;
    const auto t = Vec2::cross(c, s) / rxs;
    const auto u = Vec2::cross(c, ray.direction) / rxs;

    if (0 <= t && 0 <= u && u <= 1) {
        return segment.v0 + (segment.v1 - segment.v0) * u;
    }
    return std::nullopt;
}

Vec2 QuantizedLine2D::quantize(const Vec2& original) {
    return {
        std::round(original.x / EPSILON) * EPSILON,
        std::round(original.y / EPSILON) * EPSILON};
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

    const auto makeQuantized2D = [](const Vec3& a, const Vec3& b) {
        return QuantizedLine2D{toVec2(a), toVec2(b)};
    };

    // Valid cases:
    // If two are on and one is above, the line between the two is returned.
    if (classified.onCount == 2 && classified.aboveCount == 1) {
        return makeQuantized2D(classified.getPointOnZ(0), classified.getPointOnZ(1));
    }
    // If one is on and one is above and one is below, the line between the on-one and an intersection is returned.
    if (classified.onCount == 1 && classified.belowCount == 1 && classified.aboveCount == 1) {
        const auto intersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        return makeQuantized2D(classified.getPointOnZ(0), intersection);
    }
    // If one is above and two are below, the two intersections are returned.
    if (classified.aboveCount == 1 && classified.belowCount == 2) {
        const auto firstIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        const auto secondIntersection = getIntersectionOrThrow(classified.getPointBelowZ(1), classified.getPointAboveZ(0), zPosition);
        return makeQuantized2D(firstIntersection, secondIntersection);
    }
    // If one is below and two are above, the two intersections are returned.
    if (classified.belowCount == 1 && classified.aboveCount == 2) {
        const auto firstIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(0), zPosition);
        const auto secondIntersection = getIntersectionOrThrow(classified.getPointBelowZ(0), classified.getPointAboveZ(1), zPosition);
        return makeQuantized2D(firstIntersection, secondIntersection);
    }

    throw std::runtime_error("Unhandled triangle intersection case.");
}

std::vector<QuantizedLine2D> intersect(std::span<const Triangle3D> triangles, float zPosition) {
    auto result = std::vector<QuantizedLine2D>{};
    for (const auto& triangle : triangles) {
        if (auto intersection = intersect(triangle, zPosition)) {
            result.emplace_back(*intersection);
        }
    }
    return result;
}


}