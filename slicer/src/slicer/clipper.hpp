#pragma once

#include <optional>

#include "slicer/geometry.hpp"

namespace slicer {

//! Which part of the triangle to keep after clipping at zPosition.
enum class KeepRegion { Above = 0, Below = 1 };

namespace detail {

//! Returns the point at which the line described by p0 and p1 intersects yPosition.
[[nodiscard]] Vec3 intersect(const Vec3& p0, const Vec3& p1, float zPosition);

//! Describes the "behavior" of the line from p0 to p1, with respect to some zPosition.
enum class LineBehavior {
    RemainsIn,  // This is inclusive. The line described by p0 and p1 where both equal z RemainsIn.
    Exits,      // p0 is inclusive. If p0 equals z and p1 is out, the line Exits.
    RemainsOut, // This is exclusive.
    Enters      // p1 is exclusive.
};

[[nodiscard]] LineBehavior lineBehavior(const Vec3& p0, const Vec3& p1, float zPosition, KeepRegion keepRegion);

}

//! Assumes polygon is closed and convex (intersects zPosition 0 or 2 times).
[[nodiscard]] Polygon3D clip(const Polygon3D& polygon, float zPosition, KeepRegion keepRegion);

}