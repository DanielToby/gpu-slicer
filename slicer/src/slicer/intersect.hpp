#pragma once

#include <optional>
#include <set>

#include "slicer/geometry.hpp"

namespace slicer {

//! Snaps points to an integer grid.
struct QuantizedVec2 {
    using ValueType = int64_t;

    ValueType qx;
    ValueType qy;

    QuantizedVec2() = default;
    QuantizedVec2(float x, float y);
    QuantizedVec2(ValueType qx, ValueType qy) : qx(qx), qy(qy) {}

    [[nodiscard]] Vec2 toVec2() const noexcept;

    bool operator==(const QuantizedVec2& o) const noexcept {
        return qx == o.qx && qy == o.qy;
    }

    [[nodiscard]] bool operator<(const QuantizedVec2& other) const noexcept {
        if (qx < other.qx) {
            return true;
        }
        if (qx == other.qx) {
            return qy < other.qy;
        }
        return false;
    }
};

//! Enforces a canonical ordering of segment points.
struct QuantizedSegment2D {
    QuantizedVec2 v0;
    QuantizedVec2 v1;

    [[nodiscard]] bool operator==(const QuantizedSegment2D& other) const noexcept {
        return v0 == other.v0 && v1 == other.v1;
    }

    [[nodiscard]] bool operator<(const QuantizedSegment2D& other) const noexcept {
        if (v0 < other.v0) {
            return true;
        }
        if (v0 == other.v0) {
            return v1 < other.v1;
        }
        return false;
    }
};

[[nodiscard]] bool intersects(const Triangle3D& triangle, float zPosition);

[[nodiscard]] std::optional<Vec3> intersect(const Segment3D& segment, float zPosition);

//! Intersects the triangle with the plane described by zPosition, and adds the results to data.
//! this function's implementation describes the rules by which we keep intersections with triangles lying directly on the z-position.
[[nodiscard]] std::optional<Segment3D> intersect(const Triangle3D& triangle, float zPosition);

//! Returns all segments produced by intersecting triangles with zPosition.
std::set<QuantizedSegment2D> intersect(std::span<const Triangle3D> triangles, float zPosition);

std::ostream& operator<<(std::ostream& os, const QuantizedVec2& v);

std::ostream& operator<<(std::ostream& os, const QuantizedSegment2D& v);

}