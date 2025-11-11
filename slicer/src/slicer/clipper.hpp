#pragma once

#include <optional>

#include "slicer/geometry.hpp"

namespace slicer {

namespace detail {

//! Returns the point at which the line described by p0 and p1 intersects yPosition.
[[nodiscard]] Vec3 intersect(Vec3 p0, Vec3 p1, float zPosition);

}

//! Which part of the triangle to keep after clipping at zPosition.
enum class KeepRegion { Above = 0, Below = 1 };

//! Assumes polygon is closed and convex (intersects zPosition 0 or 2 times).
[[nodiscard]] Polygon3D clip(const Polygon3D& polygon, float zPosition, KeepRegion keepRegion);

}