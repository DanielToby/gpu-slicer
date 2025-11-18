#pragma once

#include <unordered_set>

#include "slicer/geometry.hpp"

namespace slicer {

//! Snaps points to an integer grid.
struct QuantizedVec2 {
    using ValueType = int64_t;

    ValueType qx;
    ValueType qy;

    [[nodiscard]] Vec2 toVec2() const noexcept;

    bool operator==(const QuantizedVec2& o) const noexcept {
        return qx == o.qx && qy == o.qy;
    }

    [[nodiscard]] QuantizedVec2 operator-(const QuantizedVec2& other) const noexcept {
        return {qx - other.qx, qy - other.qy};
    }

    [[nodiscard]] QuantizedVec2 operator+(const QuantizedVec2& other) const noexcept {
        return {qx + other.qx, qy + other.qy};
    }

    [[nodiscard]] QuantizedVec2 operator*(ValueType s) const noexcept {
        return {qx * s, qy * s};
    }
};

//! Enforces a canonical ordering of segment points.
struct QuantizedSegment2D {
    QuantizedVec2 v0;
    QuantizedVec2 v1;

    bool operator==(const QuantizedSegment2D& other) const noexcept {
        return v0 == other.v0 && v1 == other.v1;
    }
};

struct QuantizedVec2Hash {
    std::size_t operator()(const QuantizedVec2& v) const noexcept;
};

//! Swaps the order before hashing, if necessary.
struct BidirectionalQuantizedSegment2DHash {
    std::size_t operator()(const QuantizedSegment2D& s) const noexcept;
};

//! This type-safe guarantee of quantized, deduplicated points / edges helps ensure that our mesh is manifold and that
//! our construction of the adjacency list goes smoothly.
struct IntersectData {
    std::unordered_set<QuantizedVec2, QuantizedVec2Hash> vertices;
    std::unordered_set<QuantizedSegment2D, BidirectionalQuantizedSegment2DHash> edges;
};

[[nodiscard]] std::optional<Vec3> intersect(const Segment3D& segment, float zPosition);

//! Intersects the triangle with the plane described by zPosition, and adds the results to data.
//! this function's implementation describes the rules by which we keep intersections with triangles lying directly on the z-position.
[[nodiscard]] std::optional<Segment3D> intersect(const Triangle3D& triangle, float zPosition);

//! Returns all data produced from intersecting triangles with zPosition.
IntersectData intersect(std::span<const Triangle3D> triangles, float zPosition);

std::ostream& operator<<(std::ostream& os, const QuantizedVec2& v);

std::ostream& operator<<(std::ostream& os, const QuantizedSegment2D& v);

}