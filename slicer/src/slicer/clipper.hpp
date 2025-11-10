#pragma once

#include "slicer/geometry.hpp"

namespace slicer {

//! Which part of the triangle to keep after clipping at zPosition.
enum class KeepRegion { Above = 0, Below = 1 };

//! Assumes polygon is closed and convex (intersects zPosition 0 or 2 times).
[[nodiscard]] Polygon3D clip(const Polygon3D& polygon, float zPosition, KeepRegion keepRegion);

}