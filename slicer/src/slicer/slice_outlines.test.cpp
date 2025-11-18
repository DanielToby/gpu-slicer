#include <catch2/catch_test_macros.hpp>

#include "slicer/slice_outlines.hpp"

using namespace slicer;


TEST_CASE("getSliceOutlines: one outline") {
    auto adjacencyList = ManifoldAdjacencyList{};
    adjacencyList[{0, 0}] = std::array{QuantizedVec2{1, 1}, QuantizedVec2{2, 2}};
    adjacencyList[{1, 1}] = std::array{QuantizedVec2{0, 0}, QuantizedVec2{2, 2}};
    adjacencyList[{2, 2}] = std::array{QuantizedVec2{0, 0}, QuantizedVec2{1, 1}};

    auto outlines = getSliceOutlines(adjacencyList);
    REQUIRE(outlines.size() == 1);
    CHECK(outlines[0].size() == 3);
    CHECK(outlines[0] == SliceOutline{{2, 2}, {0, 0}, {1, 1}});
}

TEST_CASE("getSliceOutlines: two outlines") {
    auto adjacencyList = ManifoldAdjacencyList{};

    // Outline 1
    adjacencyList[{0, 0}] = std::array{QuantizedVec2{1, 1}, QuantizedVec2{2, 2}};
    adjacencyList[{1, 1}] = std::array{QuantizedVec2{0, 0}, QuantizedVec2{2, 2}};
    adjacencyList[{2, 2}] = std::array{QuantizedVec2{0, 0}, QuantizedVec2{1, 1}};

    // Outline 2
    adjacencyList[{3, 3}] = std::array{QuantizedVec2{4, 4}, QuantizedVec2{6, 6}};
    adjacencyList[{4, 4}] = std::array{QuantizedVec2{5,5}, QuantizedVec2{3, 3}};
    adjacencyList[{5, 5}] = std::array{QuantizedVec2{6, 6}, QuantizedVec2{4, 4}};
    adjacencyList[{6, 6}] = std::array{QuantizedVec2{3, 3}, QuantizedVec2{5, 5}};

    auto outlines = getSliceOutlines(adjacencyList);
    REQUIRE(outlines.size() == 2);
    CHECK(outlines[0].size() == 4);
    CHECK(outlines[0] == SliceOutline{{6, 6}, {3, 3}, {4, 4}, {5, 5}});

    CHECK(outlines[1].size() == 3);
    CHECK(outlines[1] == SliceOutline{ {1, 1}, {2, 2}, {0, 0}});
}

TEST_CASE("identifyOutlineWindings: positive outline area") {
    auto positiveOutline = SliceOutline{{0, 0}, {1, 0}, {0, 1}};
    auto result = identifyWindings({positiveOutline});

    REQUIRE(result.size() == 1);
    const auto& outlineAndWinding = result[0];
    CHECK(outlineAndWinding.outline == positiveOutline);
    CHECK(outlineAndWinding.getWinding() == Winding::CCW);
}

TEST_CASE("identifyOutlineWindings: negative outline area") {
    auto negativeOutline = SliceOutline{{0, 0}, {0, 1}, {1, 0}};
    auto result = identifyWindings({negativeOutline});

    REQUIRE(result.size() == 1);
    const auto& outlineAndWinding = result[0];
    CHECK(outlineAndWinding.outline == negativeOutline);
    CHECK(outlineAndWinding.getWinding() == Winding::CW);
}
