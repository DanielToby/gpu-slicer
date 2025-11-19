#include "slicer/adjacency_list.hpp"

#include <iostream>

namespace slicer {

using AdjacencyList = std::map<QuantizedVec2, std::array<std::optional<QuantizedVec2>, 2>>;

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

    for (const auto& [vertex, adjacencies] : adjacencyList) {
        if (adjacencies[0] && adjacencies[1]) {
            result[vertex] = std::array{*adjacencies[0], *adjacencies[1]};
        } else {
            throw std::runtime_error("Mesh is not manifold.");
        }
    }
    return result;
}

ManifoldAdjacencyList getManifoldAdjacencyList(const std::set<QuantizedSegment2D>& segments) {
    AdjacencyList result;
    for (const auto& [v0, v1] : segments) {
        if (v0 != v1) {
            addOrThrowIfFull(result[v0], v1);
            addOrThrowIfFull(result[v1], v0);
        } else {
            std::cout << "Discarding edge smaller than EPSILON." << std::endl;
        }
    }

    return getManifoldOrThrow(result);
}

}