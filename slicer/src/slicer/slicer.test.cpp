#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <iostream>

#include "slicer/slicer.hpp"
#include "slicer/stl.hpp"
#include "slicer/svg.hpp"
#include "spatial_index/no_spatial_index.hpp"

namespace {

template <typename Fn>
void logTimedExecution(std::string&& message, Fn&& fn) {
    using namespace std::chrono;
    const auto start = high_resolution_clock::now();

    std::invoke(fn);

    const auto stop = high_resolution_clock::now();
    const auto duration = duration_cast<milliseconds>(stop - start);
    std::cout << message << " took: " << duration.count() << " ms" << std::endl;
}


[[nodiscard, maybe_unused]] std::string toString(const slicer::Vec2& pt) {
    return "{" + std::to_string(pt.x) + ", " + std::to_string(pt.y) + "}";
}

void printOutline(const slicer::SliceOutline& outline) {
    std::string result = "[ ";
    for (const auto& entry : outline) {
        result += toString(entry) + ",  ";
    }
    result += "]";
    std::cout << result << std::endl;
}

}

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
//       only the ordering is deterministic.

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

TEST_CASE("slice") {
    auto triangles = slicer::loadStl("/Users/daniel.toby/Desktop/3DBenchy.stl");

    // min: -29.176 -15.502 0
    // max: 30.825 15.502 48
    // z span: [0, 48]

    auto noSpatialIndex = slicer::NoSpatialIndex{};
    logTimedExecution("Build spatial index (none)", [&] {
        noSpatialIndex.build(triangles);
    });

    std::vector<slicer::Slice> slices;
    logTimedExecution("Slice (no spatial index)", [&] {
        slices = slicer::slice(noSpatialIndex, .5);
    });

    auto bbox = noSpatialIndex.AABB();
    auto dimensions = slicer::BBox2D{slicer::toVec2(bbox.min), slicer::toVec2(bbox.max)};
    for (auto i = 0; i < slices.size(); i++) {
        auto path = "/Users/daniel.toby/Desktop/output/" + std::to_string(i) + ".svg";
        slicer::writeSVG(dimensions, slices[i].polygons, path);
    }
}
