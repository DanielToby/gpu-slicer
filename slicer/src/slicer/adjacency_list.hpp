#pragma once

#include <unordered_map>
#include <unordered_set>

#include "slicer/geometry.hpp"

namespace slicer {

//! Produces an adjacency list from the collection of lines, then enforces that it's manifold (throws if not).
using ManifoldAdjacencyList = std::unordered_map<Vec2, std::array<Vec2, 2>, Vec2Hash>;
[[nodiscard]] ManifoldAdjacencyList getManifoldAdjacencyList(std::span<const QuantizedLine2D> lines) noexcept(false);

}