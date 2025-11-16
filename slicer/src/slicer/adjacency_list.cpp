#include "slicer/adjacency_list.hpp"

namespace slicer {

using AdjacencyList = std::unordered_map<Vec2, std::array<std::optional<Vec2>, 2>, Vec2Hash>;

void addOrThrowIfFull(std::array<std::optional<Vec2>, 2>& list, Vec2 value) {
    if (!list[0]) {
        list[0] = value;
    } else if (!list[1]) {
        list[1] = value;
    } else {
        throw std::runtime_error("Mesh is not manifold.");
    }
}

ManifoldAdjacencyList getManifoldOrThrow(const AdjacencyList& adjacencyList) {
    auto result = ManifoldAdjacencyList{};
    result.reserve(adjacencyList.size());

    for (const auto& [vertex, adjacencies] : adjacencyList) {
        if (adjacencies[0] && adjacencies[1]) {
            result[vertex] = std::array{*adjacencies[0], *adjacencies[1]};
        } else {
            throw std::runtime_error("Mesh is not manifold.");
        }
    }
    return result;
}

ManifoldAdjacencyList getManifoldAdjacencyList(const IntersectData& intersections) {
    AdjacencyList result;
    result.reserve(intersections.vertices.size());

    for (const auto& edge : intersections.edges) {
        addOrThrowIfFull(result[edge.v0.value()], edge.v1.value());
        addOrThrowIfFull(result[edge.v1.value()], edge.v0.value());
    }

    return getManifoldOrThrow(result);
}

}