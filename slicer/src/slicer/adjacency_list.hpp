#pragma once

#include <unordered_map>

#include "slicer/intersect.hpp"

namespace slicer {

//! Produces an adjacency list from the collection of lines, then enforces that it's manifold (throws if not).
using ManifoldAdjacencyList = std::unordered_map<QuantizedVec2, std::array<QuantizedVec2, 2>, QuantizedVec2Hash>;
[[nodiscard]] ManifoldAdjacencyList getManifoldAdjacencyList(const IntersectData& intersections) noexcept(false);

}