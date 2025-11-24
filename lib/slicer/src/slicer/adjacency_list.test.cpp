#include <catch2/catch_test_macros.hpp>

#include "slicer/adjacency_list.hpp"

using namespace slicer;

TEST_CASE("buildAdjacencyList, buildOutlines: one outline") {
    auto oneOutline = std::set<QuantizedSegment2D>{
        {QuantizedVec2{0.f, 0.f}, QuantizedVec2{1.f, 1.f}},
        {QuantizedVec2{1.f, 1.f}, QuantizedVec2{2.f, 2.f}},
        {QuantizedVec2{2.f, 2.f}, QuantizedVec2{0.f, 0.f}}};
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.size());
}

TEST_CASE("buildAdjacencyList, buildOutlines: two outlines") {
    auto oneOutline = std::set<QuantizedSegment2D>{
        {QuantizedVec2{0.f, 0.f}, QuantizedVec2{1.f, 1.f}},// Outline 1
        {QuantizedVec2{1.f, 1.f}, QuantizedVec2{2.f, 2.f}},
        {QuantizedVec2{2.f, 2.f}, QuantizedVec2{0.f, 0.f}},
        {QuantizedVec2{3.f, 3.f}, QuantizedVec2{4.f, 4.f}},// Outline 2
        {QuantizedVec2{4.f, 4.f}, QuantizedVec2{5.f, 5.f}},
        {QuantizedVec2{5.f, 5.f}, QuantizedVec2{6.f, 6.f}},
        {QuantizedVec2{6.f, 6.f}, QuantizedVec2{3.f, 3.f}}};
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.size());
}
