#include <catch2/catch_test_macros.hpp>

#include "slicer/adjacency_list.hpp"

using namespace slicer;

namespace {

[[nodiscard]] IntersectData mockIntersectData(const std::vector<std::array<QuantizedVec2, 2>>& edges) {
    IntersectData data;
    for (const auto& edge : edges) {
        data.vertices.insert(edge[0]);
        data.vertices.insert(edge[1]);
        data.edges.insert({edge[0], edge[1]});
    }
    return data;
}

}

TEST_CASE("buildAdjacencyList, buildOutlines: one outline") {
    auto oneOutline = mockIntersectData({
        std::array{QuantizedVec2{0, 0}, QuantizedVec2{1, 1}},
        std::array{QuantizedVec2{1, 1}, QuantizedVec2{2, 2}},
        std::array{QuantizedVec2{2, 2}, QuantizedVec2{0, 0}}});
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.vertices.size());
    CHECK(adjacencyList.size() == oneOutline.edges.size());
}

TEST_CASE("buildAdjacencyList, buildOutlines: two outlines") {
    auto oneOutline = mockIntersectData({
        {std::array{QuantizedVec2{0, 0}, QuantizedVec2{1, 1}}}, // Outline 1
        {std::array{QuantizedVec2{1, 1}, QuantizedVec2{2, 2}}},
        {std::array{QuantizedVec2{2, 2}, QuantizedVec2{0, 0}}},
        {std::array{QuantizedVec2{3, 3}, QuantizedVec2{4, 4}}}, // Outline 2
        {std::array{QuantizedVec2{4, 4}, QuantizedVec2{5, 5}}},
        {std::array{QuantizedVec2{5, 5}, QuantizedVec2{6, 6}}},
        {std::array{QuantizedVec2{6, 6}, QuantizedVec2{3, 3}}}
    });
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.vertices.size());
    CHECK(adjacencyList.size() == oneOutline.edges.size());
}
