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

namespace {

[[nodiscard]] slicer::IntersectData mockIntersectData(const std::vector<std::array<slicer::Vec2, 2>>& edges) {
    slicer::IntersectData data;
    for (const auto& edge : edges) {
        auto p0 = slicer::QuantizedPoint2D{edge[0]};
        auto p1 = slicer::QuantizedPoint2D{edge[1]};
        data.vertices.insert(p0);
        data.vertices.insert(p1);
        data.edges.emplace_back(slicer::QuantizedLine2D{p0, p1});
    }
}

}

// TODO: These tests currently assume a consistent direction and start position between outlines, but they shouldn't.
TEST_CASE("buildAdjacencyList, buildOutlines: one outline") {
    using namespace slicer;
    auto oneOutline = mockIntersectData({
        std::array{Vec2{0, 0}, Vec2{1, 1}},
        std::array{Vec2{1, 1}, Vec2{2, 2}},
        std::array{Vec2{2, 2}, Vec2{0, 0}}});
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.vertices.size());
    CHECK(adjacencyList.size() == oneOutline.edges.size());

    auto outlines = getSliceOutlines(adjacencyList);
    REQUIRE(outlines.size() == 1);
    CHECK(outlines[0].size() == 3);
    CHECK(outlines[0] == SliceOutline{{2, 2}, {0, 0}, {1, 1}});
}

TEST_CASE("buildAdjacencyList, buildOutlines: two outlines") {
    using namespace slicer;
    auto oneOutline = mockIntersectData({
        {std::array{Vec2{0, 0}, Vec2{1, 1}}}, // Outline 1
        {std::array{Vec2{1, 1}, Vec2{2, 2}}},
        {std::array{Vec2{2, 2}, Vec2{0, 0}}},
        {std::array{Vec2{3, 3}, Vec2{4, 4}}}, // Outline 2
        {std::array{Vec2{4, 4}, Vec2{5, 5}}},
        {std::array{Vec2{5, 5}, Vec2{6, 6}}},
        {std::array{Vec2{6, 6}, Vec2{3, 3}}}
    });
    auto adjacencyList = getManifoldAdjacencyList(oneOutline);
    CHECK(adjacencyList.size() == oneOutline.vertices.size());
    CHECK(adjacencyList.size() == oneOutline.edges.size());

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
    CHECK(outlineAndWinding.getWinding() == Winding::CCW);
}

TEST_CASE("identifyOutlineWindings: negative outline area") {
    using namespace slicer;
    auto negativeOutline = SliceOutline{{0, 0}, {0, 1}, {1, 0}};
    auto result = identifyWindings({negativeOutline});

    REQUIRE(result.size() == 1);
    const auto& outlineAndWinding = result[0];
    CHECK(outlineAndWinding.outline == negativeOutline);
    CHECK(outlineAndWinding.getWinding() == Winding::CW);
}
