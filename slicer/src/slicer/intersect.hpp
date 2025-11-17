#pragma once

#include <unordered_set>

#include "slicer/geometry.hpp"

namespace slicer {

//! Functions and Classes dedicated to the intersection of quantized points.
//! There is some overhead (literal and cognitive) associated with these "Quantized" wrappers around
//! our geometry types, but it guarantees that we're not building duplicates into our adjacency list.

//! Returns a quantized Vec2.
[[nodiscard]] Vec2 quantize(const Vec2& in);
[[nodiscard]] Vec3 quantize(const Vec3& in);

//! Snaps point to a grid.
template <typename PointType>
class QuantizedPoint {
    template <typename>
    // Allow access to private c'tor in other specializations; useful for as() below.
    friend class QuantizedPoint;

    explicit QuantizedPoint(const PointType& value) : m_value(value) {}

public:
    [[nodiscard]] static QuantizedPoint fromPoint(const PointType& value) {
        return QuantizedPoint{quantize(value)};
    }

    [[nodiscard]] PointType value() const { return m_value; }
    [[nodiscard]] bool operator==(const QuantizedPoint& other) const {
        return m_value == other.m_value;
    }

    //! If PointType supports conversion to another point type, so does this class, allowing callers to avoid re-quantizing.
    template <typename OtherPointType>
    [[nodiscard]] QuantizedPoint<OtherPointType> as() const {
        return QuantizedPoint<OtherPointType>{m_value.template as<OtherPointType>()};
    }

private:
    PointType m_value;
};

using QuantizedPoint2D = QuantizedPoint<Vec2>;
using QuantizedPoint3D = QuantizedPoint<Vec3>;

struct QuantizedPoint2DHash {
    std::size_t operator()(const QuantizedPoint2D& v) const noexcept;
};

template <typename PointType>
struct QuantizedLine {
    [[nodiscard]] bool operator==(const QuantizedLine& other) const {
        return v0 == other.v0 && v1 == other.v1;
    }

    QuantizedPoint<PointType> v0;
    QuantizedPoint<PointType> v1;
};

using QuantizedLine2D = QuantizedLine<Vec2>;
using QuantizedLine3D = QuantizedLine<Vec3>;

//! This is direction agnostic! The result is the same as hashing QuantizedLine2D{v1, v0}.
struct QuantizedLine2DHash {
    std::size_t operator()(const QuantizedLine2D& v) const noexcept;
};

//! Intersects the triangle with the plane described by zPosition, and adds the results to data.
//! this function's implementation describes the rules by which we keep intersections with triangles lying directly on the z-position.
[[nodiscard]] std::optional<QuantizedLine2D> intersect(const Triangle3D& triangle, float zPosition);

//! This type-safe guarantee of quantized, deduplicated points / edges helps ensure that our mesh is manifold and that
//! our construction of the adjacency list goes smoothly.
struct IntersectData {
    std::unordered_set<QuantizedPoint2D, QuantizedPoint2DHash> vertices;
    std::unordered_set<QuantizedLine2D, QuantizedLine2DHash> edges;
};

//! Adds all segments created by intersecting the provided triangles to `data`.
IntersectData intersect(std::span<const Triangle3D> triangles, float zPosition);

[[nodiscard]] bool intersects(const QuantizedLine3D& segment, float zPosition);

[[nodiscard]] std::optional<QuantizedPoint3D> intersect(const QuantizedLine3D& segment, float zPosition);

//! This is used later in the pipeline, when quantization is less important. TODO: figure out types here.
[[nodiscard]] std::optional<Vec2> intersect(const Segment2D& segment, const Ray2D& ray);

}