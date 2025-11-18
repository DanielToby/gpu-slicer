#pragma once

#include <unordered_set>

#include "slicer/geometry.hpp"

namespace slicer {

//! Intersects the triangle with the plane described by zPosition, and adds the results to data.
//! this function's implementation describes the rules by which we keep intersections with triangles lying directly on the z-position.
[[nodiscard]] std::optional<Segment2D> intersect(const Triangle3D& triangle, float zPosition);

//! This type-safe guarantee of quantized, deduplicated points / edges helps ensure that our mesh is manifold and that
//! our construction of the adjacency list goes smoothly.
struct IntersectData {
    std::unordered_set<Vec2, Vec2Hash> vertices;
    std::unordered_set<Segment2D, BidirectionalSegment2DHash> edges;
};

//! Returns all data produced from intersecting triangles with zPosition.
IntersectData intersect(std::span<const Triangle3D> triangles, float zPosition);

[[nodiscard]] std::optional<Vec3> intersect(const Segment3D& segment, float zPosition);

}