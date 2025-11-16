#include "slicer/adjacency_list.hpp"

namespace slicer {

namespace {

using AdjacencyList = std::unordered_map<Vec2, std::unordered_set<Vec2, Vec2Hash>, Vec2Hash>;

[[nodiscard]] ManifoldAdjacencyList makeManifold(const AdjacencyList& adjacencyList) noexcept(false) {
    ManifoldAdjacencyList result;
    for (const auto& [v, neighbors] : adjacencyList) {
        if (neighbors.size() != 2) {
            throw std::runtime_error("Mesh is not manifold.");
        }

        auto it = neighbors.begin();
        const auto& a = *it++;
        const auto& b = *it;

        result[v] = {a, b};
    }
    return result;
}

}

ManifoldAdjacencyList getManifoldAdjacencyList(std::span<const QuantizedLine2D> lines) {
    AdjacencyList result;
    for (const auto& line : lines) {
        result[line.v0()].insert(line.v1());
        result[line.v1()].insert(line.v0());
    }
    return makeManifold(result);
}

}