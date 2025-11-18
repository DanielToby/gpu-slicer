#include "slicer/adjacency_list.hpp"

namespace slicer {

using AdjacencyList = std::unordered_map<QuantizedVec2, std::array<std::optional<QuantizedVec2>, 2>, QuantizedVec2Hash>;

void addOrThrowIfFull(std::array<std::optional<QuantizedVec2>, 2>& list, QuantizedVec2 value) {
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
        if (edge.v0 == edge.v1) {
            throw std::runtime_error("Empty intersection.");
        }

        addOrThrowIfFull(result[edge.v0], edge.v1);
        addOrThrowIfFull(result[edge.v1], edge.v0);
    }

    return getManifoldOrThrow(result);
}

}