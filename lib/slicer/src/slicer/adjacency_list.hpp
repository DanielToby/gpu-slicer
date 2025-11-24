#pragma once

#include <map>

#include "geometry/intersect.hpp"

namespace slicer {

//! Produces an adjacency list from the collection of lines, then enforces that it's manifold (throws if not).
using ManifoldAdjacencyList = std::map<QuantizedVec2, std::array<QuantizedVec2, 2>>;
[[nodiscard]] ManifoldAdjacencyList getManifoldAdjacencyList(const std::set<QuantizedSegment2D>& segments) noexcept(false);

}