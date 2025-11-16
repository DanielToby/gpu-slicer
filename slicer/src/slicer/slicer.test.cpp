#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <iostream>

#include "slicer/slicer.hpp"
#include "slicer/stl.hpp"
#include "slicer/svg.hpp"
#include "spatial_index/no_spatial_index.hpp"

TEST_CASE("getSliceHeights: thickness smaller than bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 1.) == std::vector<float>{-2, -1, 0, 1});
}

TEST_CASE("getSliceHeights: thickness equal to bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 4.) == std::vector<float>{-2});
}

TEST_CASE("getSliceHeights: thickness larger than bbox height") {
    auto bbox = slicer::BBox3D{{-2, -2, -2}, {2, 2, 2}};
    CHECK(slicer::getSliceHeights(bbox, 5.) == std::vector<float>{-2});
}

// TODO: These tests currently assume a consistent direction and start position between outlines, but they shouldn't.
TEST_CASE("buildAdjacencyList, buildOutlines: one outline") {
    using namespace slicer;
    auto oneOutline = std::vector{
        QuantizedLine2D{{0, 0}, {1, 1}},
        QuantizedLine2D{{1, 1}, {2, 2}},
        QuantizedLine2D{{2, 2}, {0, 0}}};
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.size());

    auto outlines = getSliceOutlines(adjacencyList);
    REQUIRE(outlines.size() == 1);
    CHECK(outlines[0].size() == 3);
    CHECK(outlines[0] == SliceOutline{{2, 2}, {0, 0}, {1, 1}});
}

TEST_CASE("buildAdjacencyList, buildOutlines: two outlines") {
    using namespace slicer;
    auto oneOutline = std::vector{
        QuantizedLine2D{{0, 0}, {1, 1}}, // Outline 1
        QuantizedLine2D{{1, 1}, {2, 2}},
        QuantizedLine2D{{2, 2}, {0, 0}},
        QuantizedLine2D{{3, 3}, {4, 4}}, // Outline 2
        QuantizedLine2D{{4, 4}, {5, 5}},
        QuantizedLine2D{{5, 5}, {6, 6}},
        QuantizedLine2D{{6, 6}, {3, 3}},
    };
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.size());

    auto outlines = getSliceOutlines(adjacencyList);
    REQUIRE(outlines.size() == 2);
    CHECK(outlines[0].size() == 4);
    CHECK(outlines[0] == SliceOutline{{6, 6}, {3, 3}, {4, 4}, {5, 5}});

    CHECK(outlines[1].size() == 3);
    CHECK(outlines[1] == SliceOutline{ {1, 1}, {2, 2}, {0, 0}});
}

TEST_CASE("identifyOutlineWindings: positive outline area") {
    using namespace slicer;
    auto positiveOutline = SliceOutline{{0, 0}, {1, 0}, {0, 1}};
    auto result = identifyWindings({positiveOutline});

    REQUIRE(result.size() == 1);
    const auto& outlineAndWinding = result[0];
    CHECK(outlineAndWinding.outline == positiveOutline);
    CHECK(outlineAndWinding.relativeWinding == RelativeWinding::Positive);
}

TEST_CASE("identifyOutlineWindings: negative outline area") {
    using namespace slicer;
    auto negativeOutline = SliceOutline{{0, 0}, {0, 1}, {1, 0}};
    auto result = identifyWindings({negativeOutline});

    REQUIRE(result.size() == 1);
    const auto& outlineAndWinding = result[0];
    CHECK(outlineAndWinding.outline == negativeOutline);
    CHECK(outlineAndWinding.relativeWinding == RelativeWinding::Negative);
}
